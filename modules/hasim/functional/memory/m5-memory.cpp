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
#include "sim/faults.hh"
#include "sim/eventq.hh"


FUNCP_SIMULATED_MEMORY_CLASS::FUNCP_SIMULATED_MEMORY_CLASS()
{
    SetTraceableName("funcp_memory_m5");

    memPort = &(M5Cpu(0)->tc->getCpuPtr()->getDataPort());

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
}


bool
FUNCP_SIMULATED_MEMORY_CLASS::Read(
    UINT64 paddr,
    UINT64 size,
    bool isSpeculative,
    void *dest)
{
    ASSERTX(size > 0);

    return BlobHelper(paddr, (uint8_t*)dest, size, MemCmd::ReadReq, isSpeculative);
}


void
FUNCP_SIMULATED_MEMORY_CLASS::Write(
    UINT64 paddr,
    UINT64 size,
    void *src)
{
    ASSERTX(size > 0);

    BlobHelper(paddr, (uint8_t*)src, size, MemCmd::WriteReq, false);
}


//
// BlobHelper is a standard m5 style, such as in mem/port.cc, for sending a
// request to physical memory.
//
bool
FUNCP_SIMULATED_MEMORY_CLASS::BlobHelper(
    Addr paddr,
    uint8_t *p,
    int size,
    MemCmd cmd,
    bool isSpeculative)
{
    bool success = true;

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

        return success;
    }

    Request req;

    for (ChunkGenerator gen(paddr, size, TheISA::PageBytes);
         ! gen.done(); gen.next())
    {
        curEventQueue(mainEventQueue[0]);
        req.setPhys(gen.addr(), gen.size(), 0, Request::funcMasterId);
        Packet pkt(&req, cmd);
        pkt.dataStatic(p);
        memPort->sendFunctional(&pkt);

        success = success && ! pkt.isError();

        if (! isSpeculative)
        {
            ASSERT(! pkt.isError(), "m5 memory error " <<
                                     (cmd == MemCmd::WriteReq ? "WRITE" : "READ") <<
                                     " PA 0x" << fmt_x(gen.addr()));
        }

        p += gen.size();
    }

    return success;
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

    Process *proc = M5Cpu(ctxId)->tc->getProcessPtr();
    PageTable *pTable = proc->pTable;

    Addr va_page = roundDown(va, TheISA::VMPageSize);
    if (! pTable->translate(va_page, paddr))
    {
        T1("\tfuncp_memory_m5: VtoP no mapping VA " << fmt_va(va_page));

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

        proc->allocateMem(va_page, TheISA::VMPageSize);
        VERIFY(pTable->translate(va_page, paddr),
               "VtoP failed: unable to allocate page for VA 0x" << fmt_x(va_page));

        T1("\tfuncp_memory_m5: VtoP alloc VA " << fmt_va(va_page) << " to PA " << fmt_va(paddr));
    }

    FUNCP_MEM_VTOP_RESP resp;
    resp.pa = paddr;
    resp.pageFault = false;
    resp.ioSpace = false;
    return resp;
}
