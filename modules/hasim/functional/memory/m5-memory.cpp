//
// Copyright (C) 2008 Intel Corporation
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
 
//
// @file m5_memory.cpp
// @brief ISA-independent m5 memory access
// @author Michael Adler
//

#include "asim/syntax.h"
#include "asim/mesg.h"
#include "asim/atomic.h"

#include "asim/provides/funcp_base_types.h"
#include "asim/provides/funcp_simulated_memory.h"

// m5 includes
#include "base/chunk_generator.hh"


FUNCP_SIMULATED_MEMORY_CLASS::FUNCP_SIMULATED_MEMORY_CLASS()
{
    mem_port = M5Cpu(0)->tc->getMemPort();
    pTable = M5Cpu(0)->tc->getProcessPtr()->pTable;

    //
    // Allocate a guard page at 0.  m5 doesn't do this by default.  It would be
    // nice if it got physical address 0, but this is better than nothing.
    // Put the guard page as the highest page in physical memory.
    //
    if (FUNCP_ISA_P_ADDR_SIZE == sizeof(Addr))
    {
        guard_page = Addr(-1);
    }
    else
    {
        guard_page = (Addr(1) << FUNCP_ISA_P_ADDR_SIZE) - 1;
    }

    guard_page &= TheISA::PageMask;
}


FUNCP_SIMULATED_MEMORY_CLASS::~FUNCP_SIMULATED_MEMORY_CLASS()
{
    delete mem_port;
}


void
FUNCP_SIMULATED_MEMORY_CLASS::Read(
    UINT64 paddr,
    UINT64 size,
    void *dest)
{
    ASSERTX(size > 0);

    BlobHelper(paddr, (uint8_t*)dest, size, MemCmd::ReadReq);
}


void
FUNCP_SIMULATED_MEMORY_CLASS::Write(
    UINT64 paddr,
    UINT64 size,
    void *src)
{
    ASSERTX(size > 0);

    BlobHelper(paddr, (uint8_t*)src, size, MemCmd::WriteReq);
}


//
// BlobHelper is a standard m5 style, such as in mem/port.cc, for sending a
// request to physical memory.
//
void
FUNCP_SIMULATED_MEMORY_CLASS::BlobHelper(
    Addr paddr,
    uint8_t *p,
    int size,
    MemCmd cmd)
{
    if ((paddr & TheISA::PageMask) == guard_page)
    {
        //
        // Reference to virtual page 0.
        if (cmd == MemCmd::WriteReq)
        {
            ASIMERROR("Attempted write to virtual page 0!");
        }

        // Just return 0.
        bzero(p, size);

        return;
    }

    Request req;

    for (ChunkGenerator gen(paddr, size, mem_port->peerBlockSize());
         ! gen.done(); gen.next())
    {
        req.setPhys(gen.addr(), gen.size(), 0);
        Packet pkt(&req, cmd, Packet::Broadcast);
        pkt.dataStatic(p);
        mem_port->sendFunctional(&pkt);
        p += gen.size();
    }
}


//
// Virtual to physical mapping
//
UINT64
FUNCP_SIMULATED_MEMORY_CLASS::VtoP(UINT64 va)
{
    Addr paddr;

    if ((va & TheISA::PageMask) == 0)
    {
        return guard_page | (va & TheISA::PageMask);
    }

    if (! pTable->translate(va, paddr)) {
        pTable->allocate(roundDown(va, TheISA::VMPageSize), TheISA::VMPageSize);

        // For now just fail.  The functional model treats setting bit 0 of the
        // response as a translation failed flag, so we could signal a failure
        // back to the model.
        VERIFY(pTable->translate(va, paddr), "Page translation failed");
    }
   
    return paddr;
}
