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


FUNCP_SIMULATED_MEMORY_CLASS::FUNCP_SIMULATED_MEMORY_CLASS()
{
    mem_port = M5Cpu(0)->tc->getMemPort();
    pTable = M5Cpu(0)->tc->getProcessPtr()->pTable;
}


FUNCP_SIMULATED_MEMORY_CLASS::~FUNCP_SIMULATED_MEMORY_CLASS()
{
    delete mem_port;
}


void
FUNCP_SIMULATED_MEMORY_CLASS::Read(
    UINT64 addr,
    UINT64 size,
    void *dest)
{
    Fault fault;

    ASSERTX(size > 0);

    if (addr & (size - 1))
    {
        // Unaligned
        if (! mem_port->tryReadBlob(addr, (uint8_t*)dest, size))
        {
            bzero(dest, size);
        }
        return;
    }

    //
    // First try doing the read directly through the CPU.  This is a
    // faster path through m5.
    //
    while (1)
    {
        switch (size)
        {
          case 8:
            fault = M5Cpu(0)->read(addr, *(UINT64*)dest, 0);
            break;
          case 4:
            fault = M5Cpu(0)->read(addr, *(UINT32*)dest, 0);
            break;
          case 2:
            fault = M5Cpu(0)->read(addr, *(UINT16*)dest, 0);
            break;
          case 1:
            fault = M5Cpu(0)->read(addr, *(UINT8*)dest, 0);
            break;
          default:
            cerr << "Attempted to read " << size << " bytes" << endl;
            ASIMERROR("Unsupport memory read size");
            break;
        }

        if (fault == NoFault) return;

        //
        // If a translation exists for the address then invoke the fault
        // handler to add the address to the DTB.  Otherwise, use the
        // remote_gdb style interface and try to get the page allocated.
        //
        Addr paddr;
        if (pTable->translate(addr, paddr))
        {
            fault->invoke(M5Cpu(0)->tc);
        }
        else
        {
            // Last resort for allocating the page.  Otherwise, just return 0.
            if (! mem_port->tryReadBlob(addr, (uint8_t*)dest, size))
            {
                bzero(dest, size);
            }
            return;
        }
    }
}


void
FUNCP_SIMULATED_MEMORY_CLASS::Write(
    UINT64 addr,
    UINT64 size,
    void *src)
{
    Fault fault;

    ASSERTX(size > 0);

    if (addr & (size - 1))
    {
        // Unaligned
        mem_port->writeBlob(addr, (uint8_t*)src, size);
        return;
    }

    //
    // First try doing the write directly through the CPU.  This is a
    // faster path through m5.
    //
    while (1)
    {
        switch (size)
        {
          case 8:
            fault = M5Cpu(0)->write(*(UINT64*)src, addr, 0, NULL);
            break;
          case 4:
            fault = M5Cpu(0)->write(*(UINT32*)src, addr, 0, NULL);
            break;
          case 2:
            fault = M5Cpu(0)->write(*(UINT16*)src, addr, 0, NULL);
            break;
          case 1:
            fault = M5Cpu(0)->write(*(UINT8*)src, addr, 0, NULL);
            break;
          default:
            cerr << "Attempted to write " << size << " bytes" << endl;
            ASIMERROR("Unsupport memory write size");
            break;
        }
    
        if (fault == NoFault) return;

        //
        // If a translation exists for the address then invoke the fault
        // handler to add the address to the DTB.  Otherwise, use the
        // remote_gdb style interface and try to get the page allocated.
        //
        Addr paddr;
        if (pTable->translate(addr, paddr))
        {
            fault->invoke(M5Cpu(0)->tc);
        }
        else
        {
            // Last resort for writing to the page.
            mem_port->writeBlob(addr, (uint8_t*)src, size);
        }
    }
}
