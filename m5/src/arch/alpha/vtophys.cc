/*
 * Copyright (c) 2002, 2003, 2004, 2005
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
 * Authors: Nathan L. Binkert
 *          Steven K. Reinhardt
 *          Ali G. Saidi
 */

#include <string>

#include "arch/alpha/ev5.hh"
#include "arch/alpha/vtophys.hh"
#include "base/chunk_generator.hh"
#include "base/trace.hh"
#include "cpu/thread_context.hh"
#include "mem/vport.hh"

using namespace std;
using namespace AlphaISA;

AlphaISA::PageTableEntry
AlphaISA::kernel_pte_lookup(FunctionalPort *mem, Addr ptbr, AlphaISA::VAddr vaddr)
{
    Addr level1_pte = ptbr + vaddr.level1();
    AlphaISA::PageTableEntry level1 = mem->read<uint64_t>(level1_pte);
    if (!level1.valid()) {
        DPRINTF(VtoPhys, "level 1 PTE not valid, va = %#\n", vaddr);
        return 0;
    }

    Addr level2_pte = level1.paddr() + vaddr.level2();
    AlphaISA::PageTableEntry level2 = mem->read<uint64_t>(level2_pte);
    if (!level2.valid()) {
        DPRINTF(VtoPhys, "level 2 PTE not valid, va = %#x\n", vaddr);
        return 0;
    }

    Addr level3_pte = level2.paddr() + vaddr.level3();
    AlphaISA::PageTableEntry level3 = mem->read<uint64_t>(level3_pte);
    if (!level3.valid()) {
        DPRINTF(VtoPhys, "level 3 PTE not valid, va = %#x\n", vaddr);
        return 0;
    }
    return level3;
}

Addr
AlphaISA::vtophys(Addr vaddr)
{
    Addr paddr = 0;
    if (AlphaISA::IsUSeg(vaddr))
        DPRINTF(VtoPhys, "vtophys: invalid vaddr %#x", vaddr);
    else if (AlphaISA::IsK0Seg(vaddr))
        paddr = AlphaISA::K0Seg2Phys(vaddr);
    else
        panic("vtophys: ptbr is not set on virtual lookup");

    DPRINTF(VtoPhys, "vtophys(%#x) -> %#x\n", vaddr, paddr);

    return paddr;
}

Addr
AlphaISA::vtophys(ThreadContext *tc, Addr addr)
{
    AlphaISA::VAddr vaddr = addr;
    Addr ptbr = tc->readMiscRegNoEffect(AlphaISA::IPR_PALtemp20);
    Addr paddr = 0;
    //@todo Andrew couldn't remember why he commented some of this code
    //so I put it back in. Perhaps something to do with gdb debugging?
    if (AlphaISA::PcPAL(vaddr) && (vaddr < EV5::PalMax)) {
        paddr = vaddr & ~ULL(1);
    } else {
        if (AlphaISA::IsK0Seg(vaddr)) {
            paddr = AlphaISA::K0Seg2Phys(vaddr);
        } else if (!ptbr) {
            paddr = vaddr;
        } else {
            AlphaISA::PageTableEntry pte =
                kernel_pte_lookup(tc->getPhysPort(), ptbr, vaddr);
            if (pte.valid())
                paddr = pte.paddr() | vaddr.offset();
        }
    }


    DPRINTF(VtoPhys, "vtophys(%#x) -> %#x\n", vaddr, paddr);

    return paddr;
}

