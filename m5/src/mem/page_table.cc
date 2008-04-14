/* $Id$ */

/*
 * Copyright (c) 2003
 * The Regents of The University of Michigan
 * All Rights Reserved
 *
 * This code is part of the M5 simulator.
 *
 * Permission is granted to use, copy, create derivative works and
 * redistribute this software and such derivative works for any
 * purpose, so long as the copyright notice above, this grant of
 * permission, and the disclaimer below appear in all copies made; and
 * so long as the name of The University of Michigan is not used in
 * any advertising or publicity pertaining to the use or distribution
 * of this software without specific, written prior authorization.
 *
 * THIS SOFTWARE IS PROVIDED AS IS, WITHOUT REPRESENTATION FROM THE
 * UNIVERSITY OF MICHIGAN AS TO ITS FITNESS FOR ANY PURPOSE, AND
 * WITHOUT WARRANTY BY THE UNIVERSITY OF MICHIGAN OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE. THE REGENTS OF THE UNIVERSITY OF MICHIGAN SHALL NOT BE
 * LIABLE FOR ANY DAMAGES, INCLUDING DIRECT, SPECIAL, INDIRECT,
 * INCIDENTAL, OR CONSEQUENTIAL DAMAGES, WITH RESPECT TO ANY CLAIM
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OF THE SOFTWARE, EVEN
 * IF IT HAS BEEN OR IS HEREAFTER ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGES.
 *
 * Authors: Steven K. Reinhardt
 *          Ronald G. Dreslinski Jr
 *          Ali G. Saidi
 */

/**
 * @file
 * Definitions of page table.
 */
#include <string>
#include <map>
#include <fstream>

#include "arch/faults.hh"
#include "base/bitfield.hh"
#include "base/intmath.hh"
#include "base/trace.hh"
#include "mem/page_table.hh"
#include "sim/process.hh"
#include "sim/sim_object.hh"
#include "sim/system.hh"

using namespace std;
using namespace TheISA;

PageTable::PageTable(Process *_process, Addr _pageSize)
    : pageSize(_pageSize), offsetMask(mask(floorLog2(_pageSize))),
      process(_process)
{
    assert(isPowerOf2(pageSize));
    pTableCache[0].vaddr = 0;
    pTableCache[1].vaddr = 0;
    pTableCache[2].vaddr = 0;
}

PageTable::~PageTable()
{
}

void
PageTable::allocate(Addr vaddr, int64_t size)
{
    // starting address must be page aligned
    assert(pageOffset(vaddr) == 0);

    DPRINTF(MMU, "Allocating Page: %#x-%#x\n", vaddr, vaddr+ size);

    for (; size > 0; size -= pageSize, vaddr += pageSize) {
        PTableItr iter = pTable.find(vaddr);

        if (iter != pTable.end()) {
            // already mapped
            fatal("PageTable::allocate: address 0x%x already mapped",
                    vaddr);
        }

        pTable[vaddr] = TheISA::TlbEntry(process->M5_pid, vaddr,
                process->system->new_page());
        updateCache(vaddr, pTable[vaddr]);
    }
}

bool
PageTable::lookup(Addr vaddr, TheISA::TlbEntry &entry)
{
    Addr page_addr = pageAlign(vaddr);

    if (pTableCache[0].vaddr == page_addr) {
        entry = pTableCache[0].entry;
        return true;
    }
    if (pTableCache[1].vaddr == page_addr) {
        entry = pTableCache[1].entry;
        return true;
    }
    if (pTableCache[2].vaddr == page_addr) {
        entry = pTableCache[2].entry;
        return true;
    }

    PTableItr iter = pTable.find(page_addr);

    if (iter == pTable.end()) {
        return false;
    }

    updateCache(page_addr, iter->second);
    entry = iter->second;
    return true;
}

bool
PageTable::translate(Addr vaddr, Addr &paddr)
{
    TheISA::TlbEntry entry;
    if (!lookup(vaddr, entry)) {
        DPRINTF(MMU, "Couldn't Translate: %#x\n", vaddr);
        return false;
    }
    paddr = pageOffset(vaddr) + entry.pageStart();
    DPRINTF(MMU, "Translating: %#x->%#x\n", vaddr, paddr);
    return true;
}

Fault
PageTable::translate(RequestPtr req)
{
    Addr paddr;
    assert(pageAlign(req->getVaddr() + req->getSize() - 1)
           == pageAlign(req->getVaddr()));
    if (!translate(req->getVaddr(), paddr)) {
        return Fault(new GenericPageTableFault(req->getVaddr()));
    }
    req->setPaddr(paddr);
    if ((paddr & (pageSize - 1)) + req->getSize() > pageSize) {
        panic("Request spans page boundaries!\n");
        return NoFault;
    }
    return NoFault;
}

void
PageTable::serialize(std::ostream &os)
{
    paramOut(os, "ptable.size", pTable.size());

    int count = 0;

    PTableItr iter = pTable.begin();
    PTableItr end = pTable.end();
    while (iter != end) {
        os << "\n[" << csprintf("%s.Entry%d", process->name(), count) << "]\n";

        paramOut(os, "vaddr", iter->first);
        iter->second.serialize(os);

        ++iter;
        ++count;
    }
    assert(count == pTable.size());
}

void
PageTable::unserialize(Checkpoint *cp, const std::string &section)
{
    int i = 0, count;
    paramIn(cp, section, "ptable.size", count);
    Addr vaddr;
    TheISA::TlbEntry *entry;

    pTable.clear();

    while(i < count) {
        paramIn(cp, csprintf("%s.Entry%d", process->name(), i), "vaddr", vaddr);
        entry = new TheISA::TlbEntry();
        entry->unserialize(cp, csprintf("%s.Entry%d", process->name(), i));
        pTable[vaddr] = *entry;
        ++i;
   }
}

