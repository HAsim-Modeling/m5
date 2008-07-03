//
// INTEL CONFIDENTIAL
// Copyright (c) 2008 Intel Corp.  Recipient is granted a non-sublicensable 
// copyright license under Intel copyrights to copy and distribute this code 
// internally only. This code is provided "AS IS" with no support and with no 
// warranties of any kind, including warranties of MERCHANTABILITY,
// FITNESS FOR ANY PARTICULAR PURPOSE or INTELLECTUAL PROPERTY INFRINGEMENT. 
// By making any use of this code, Recipient agrees that no other licenses 
// to any Intel patents, trade secrets, copyrights or other intellectual 
// property rights are granted herein, and no other licenses shall arise by 
// estoppel, implication or by operation of law. Recipient accepts all risks 
// of use.
//
 
//
// @file m5_memory.cpp
// @brief ISA-independent m5 memory access
// @author Michael Adler
//

#include "asim/syntax.h"
#include "asim/mesg.h"
#include "asim/atomic.h"

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
    //
    if (! pTable->translate(0, guard_page)) {
        pTable->allocate(0, TheISA::VMPageSize);
        VERIFY(pTable->translate(0, guard_page), "Guard page allocation failed");
    }
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
        // Reference to page 0.
        if (cmd == MemCmd::WriteReq)
        {
            ASIMERROR("Attempted write to page 0!");
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

    if (! pTable->translate(va, paddr)) {
        pTable->allocate(roundDown(va, TheISA::VMPageSize), TheISA::VMPageSize);

        // For now just fail.  The functional model treats setting bit 0 of the
        // response as a translation failed flag, so we could signal a failure
        // back to the model.
        VERIFY(pTable->translate(va, paddr), "Page translation failed");
    }
   
    return paddr;
}
