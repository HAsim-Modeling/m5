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
#include "asim/provides/funcp_memory.h"
#include "asim/provides/funcp_simulated_memory.h"

// m5 includes
#include "base/chunk_generator.hh"


FUNCP_SIMULATED_MEMORY_CLASS::FUNCP_SIMULATED_MEMORY_CLASS()
{
    SetTraceableName("funcp_memory_m5");

    mem_port = M5Cpu(0)->tc->getMemPort();
    pTable = M5Cpu(0)->tc->getProcessPtr()->pTable;

    char fmt[16];
    sprintf(fmt, "0%dx", sizeof(MEM_VALUE) * 2);
    fmt_va = Format("0x", fmt);

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
    CONTEXT_ID ctxId,
    UINT64 paddr,
    UINT64 size,
    void *dest)
{
    ASSERTX(size > 0);

    BlobHelper(paddr, (uint8_t*)dest, size, MemCmd::ReadReq);
}


void
FUNCP_SIMULATED_MEMORY_CLASS::Write(
    CONTEXT_ID ctxId,
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
FUNCP_MEM_VTOP_RESP
FUNCP_SIMULATED_MEMORY_CLASS::VtoP(
    CONTEXT_ID ctxId,
    UINT64 va,
    bool allocOnFault)
{
    Addr paddr;

    static const char *dfault = NULL;
    static const char *dtb_miss_single = NULL;

    if ((va & TheISA::PageMask) == 0)
    {
        FUNCP_MEM_VTOP_RESP resp;
        resp.pa = guard_page | (va & TheISA::PageMask);
        resp.pageFault = false;
        resp.ioSpace = false;
        return resp;
    }

    Fault fault;
    int fault_trips = 0;
    do
    {
        fault = M5Cpu(0)->translateDataReadAddr(roundDown(va, TheISA::VMPageSize), paddr, TheISA::VMPageSize, 0);
        if (fault != NoFault)
        {
            const char *fault_name = fault->name();

            T1("\tfuncp_memory_m5: VtoP FAULT (" << fault_name << ") VA " << fmt_va(va));

            //
            // Check for fatal fault and assume it is caused by a speculative path.
            //
            if ((fault_name == dfault) || ! strcmp("dfault", fault_name))
            {
                // Remember fault name pointer to avoid using strcmp next time
                dfault = fault_name;

                if (! allocOnFault)
                {
                    // Fault returns guard page as translation so the model doesn't
                    // need too much special case code to handle faults.
                    FUNCP_MEM_VTOP_RESP resp;
                    resp.pa = guard_page | (va & TheISA::PageMask);
                    resp.pageFault = true;
                    resp.ioSpace = false;
                    return resp;
                }

                ASIMERROR("VtoP failed: unable to allocate page for VA 0x" << fmt_x(va));
            }

            fault_trips += 1;
            VERIFY(fault_trips < 10, "Too many faults translating VtoP of VA 0x" << fmt_x(va) << " in m5");

            if ((fault_name == dtb_miss_single) || ! strcmp("dtb_miss_single", fault_name))
            {
                // Remember fault name pointer to avoid using strcmp next time
                dtb_miss_single = fault_name;

                // Make sure the translation won't cause a panic before invoking
                // the handler.  This code does the early stages of
                // NDtbMissFault::invoke
                Process *p = M5Cpu(0)->tc->getProcessPtr();
                TheISA::TlbEntry entry;
                bool success = p->pTable->lookup(va, entry);
                if (! success)
                {
                    if (! allocOnFault)
                    {
                        FUNCP_MEM_VTOP_RESP resp;
                        resp.pa = guard_page | (va & TheISA::PageMask);
                        resp.pageFault = true;
                        resp.ioSpace = false;
                        return resp;
                    }

                    p->checkAndAllocNextPage(va);
                    success = p->pTable->lookup(va, entry);
                }

                if (! success)
                {
                    ASIMERROR("VtoP failed: unable to allocate page for VA 0x" << fmt_x(va));
                }
            }

            // Invoke the real handler
            fault->invoke(M5Cpu(0)->tc);
        }
    }
    while (fault != NoFault);

    FUNCP_MEM_VTOP_RESP resp;
    resp.pa = paddr;
    resp.pageFault = false;
    resp.ioSpace = false;
    return resp;
}
