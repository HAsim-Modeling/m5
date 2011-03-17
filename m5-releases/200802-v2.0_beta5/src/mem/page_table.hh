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
 */

/**
 * @file
 * Declaration of a non-full system Page Table.
 */

#ifndef __PAGE_TABLE__
#define __PAGE_TABLE__

#include <string>

#include "sim/faults.hh"
#include "arch/isa_traits.hh"
#include "arch/tlb.hh"
#include "base/hashmap.hh"
#include "mem/request.hh"
#include "sim/host.hh"
#include "sim/serialize.hh"

class Process;

/**
 * Page Table Declaration.
 */
class PageTable
{
  protected:
    typedef m5::hash_map<Addr, TheISA::TlbEntry> PTable;
    typedef PTable::iterator PTableItr;
    PTable pTable;

    struct cacheElement {
        Addr vaddr;
        TheISA::TlbEntry entry;
    };

    struct cacheElement pTableCache[3];

    const Addr pageSize;
    const Addr offsetMask;

    Process *process;

  public:

    PageTable(Process *_process, Addr _pageSize = TheISA::VMPageSize);

    ~PageTable();

    Addr pageAlign(Addr a)  { return (a & ~offsetMask); }
    Addr pageOffset(Addr a) { return (a &  offsetMask); }

    void allocate(Addr vaddr, int64_t size);

    /**
     * Lookup function
     * @param vaddr The virtual address.
     * @return entry The page table entry corresponding to vaddr.
     */
    bool lookup(Addr vaddr, TheISA::TlbEntry &entry);

    /**
     * Translate function
     * @param vaddr The virtual address.
     * @return Physical address from translation.
     */
    bool translate(Addr vaddr, Addr &paddr);

    /**
     * Perform a translation on the memory request, fills in paddr
     * field of req.
     * @param req The memory request.
     */
    Fault translate(RequestPtr req);

    /**
     * Update the page table cache.
     * @param vaddr virtual address (page aligned) to check
     * @param pte page table entry to return
     */
    inline void updateCache(Addr vaddr, TheISA::TlbEntry entry)
    {
        pTableCache[2].entry = pTableCache[1].entry;
        pTableCache[2].vaddr = pTableCache[1].vaddr;
        pTableCache[1].entry = pTableCache[0].entry;
        pTableCache[1].vaddr = pTableCache[0].vaddr;
        pTableCache[0].entry = entry;
        pTableCache[0].vaddr = vaddr;
    }


    void serialize(std::ostream &os);

    void unserialize(Checkpoint *cp, const std::string &section);
};

#endif
