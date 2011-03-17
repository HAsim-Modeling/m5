/*
 * Copyright (c) 2001, 2002, 2003, 2004, 2005
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
 * Authors: Ronald G. Dreslinski Jr
 *          Steven K. Reinhardt
 */

#include <string>
#include "base/chunk_generator.hh"
#include "mem/port.hh"
#include "mem/translating_port.hh"
#include "mem/page_table.hh"
#include "sim/process.hh"

using namespace TheISA;

TranslatingPort::TranslatingPort(const std::string &_name,
                                 Process *p, AllocType alloc)
    : FunctionalPort(_name), pTable(p->pTable), process(p),
      allocating(alloc)
{ }

TranslatingPort::~TranslatingPort()
{ }

bool
TranslatingPort::tryReadBlob(Addr addr, uint8_t *p, int size)
{
    Addr paddr;
    int prevSize = 0;

    for (ChunkGenerator gen(addr, size, VMPageSize); !gen.done(); gen.next()) {

        if (!pTable->translate(gen.addr(),paddr))
            return false;

        Port::readBlob(paddr, p + prevSize, gen.size());
        prevSize += gen.size();
    }

    return true;
}

void
TranslatingPort::readBlob(Addr addr, uint8_t *p, int size)
{
    if (!tryReadBlob(addr, p, size))
        fatal("readBlob(0x%x, ...) failed", addr);
}


bool
TranslatingPort::tryWriteBlob(Addr addr, uint8_t *p, int size)
{

    Addr paddr;
    int prevSize = 0;

    for (ChunkGenerator gen(addr, size, VMPageSize); !gen.done(); gen.next()) {

        if (!pTable->translate(gen.addr(), paddr)) {
            if (allocating == Always) {
                pTable->allocate(roundDown(gen.addr(), VMPageSize),
                                 VMPageSize);
            } else if (allocating == NextPage) {
                // check if we've accessed the next page on the stack
                if (!process->checkAndAllocNextPage(gen.addr()))
                    panic("Page table fault when accessing virtual address %#x "
                            "during functional write\n", gen.addr());
            } else {
                return false;
            }
            pTable->translate(gen.addr(), paddr);
        }

        Port::writeBlob(paddr, p + prevSize, gen.size());
        prevSize += gen.size();
    }

    return true;
}


void
TranslatingPort::writeBlob(Addr addr, uint8_t *p, int size)
{
    if (!tryWriteBlob(addr, p, size))
        fatal("writeBlob(0x%x, ...) failed", addr);
}

bool
TranslatingPort::tryMemsetBlob(Addr addr, uint8_t val, int size)
{
    Addr paddr;

    for (ChunkGenerator gen(addr, size, VMPageSize); !gen.done(); gen.next()) {

        if (!pTable->translate(gen.addr(), paddr)) {
            if (allocating == Always) {
                pTable->allocate(roundDown(gen.addr(), VMPageSize),
                                 VMPageSize);
                pTable->translate(gen.addr(), paddr);
            } else {
                return false;
            }
        }

        Port::memsetBlob(paddr, val, gen.size());
    }

    return true;
}

void
TranslatingPort::memsetBlob(Addr addr, uint8_t val, int size)
{
    if (!tryMemsetBlob(addr, val, size))
        fatal("memsetBlob(0x%x, ...) failed", addr);
}


bool
TranslatingPort::tryWriteString(Addr addr, const char *str)
{
    Addr paddr,vaddr;
    uint8_t c;

    vaddr = addr;

    do {
        c = *str++;
        if (!pTable->translate(vaddr++,paddr))
            return false;

        Port::writeBlob(paddr, &c, 1);
    } while (c);

    return true;
}

void
TranslatingPort::writeString(Addr addr, const char *str)
{
    if (!tryWriteString(addr, str))
        fatal("writeString(0x%x, ...) failed", addr);
}

bool
TranslatingPort::tryReadString(std::string &str, Addr addr)
{
    Addr paddr,vaddr;
    uint8_t c;

    vaddr = addr;

    do {
        if (!pTable->translate(vaddr++,paddr))
            return false;

        Port::readBlob(paddr, &c, 1);
        str += c;
    } while (c);

    return true;
}

void
TranslatingPort::readString(std::string &str, Addr addr)
{
    if (!tryReadString(str, addr))
        fatal("readString(0x%x, ...) failed", addr);
}

