/*
 * Copyright (c) 2006, 2007
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

#ifndef __ARCH_MIPS_LOCKED_MEM_HH__
#define __ARCH_MIPS_LOCKED_MEM_HH__

/**
 * @file
 *
 * ISA-specific helper functions for locked memory accesses.
 */

#include "arch/isa_traits.hh"
#include "base/misc.hh"
#include "base/trace.hh"
#include "mem/request.hh"


namespace MipsISA
{
template <class XC>
inline void
handleLockedRead(XC *xc, Request *req)
{
    unsigned tid = req->getThreadNum();
    xc->setMiscRegNoEffect(LLAddr, req->getPaddr() & ~0xf, tid);
    xc->setMiscRegNoEffect(LLFlag, true, tid);
    DPRINTF(LLSC, "[tid:%i]: Load-Link Flag Set & Load-Link Address set to %x.\n",
            tid, req->getPaddr() & ~0xf);
}


template <class XC>
inline bool
handleLockedWrite(XC *xc, Request *req)
{
    unsigned tid = req->getThreadNum();

    if (req->isUncacheable()) {
        // Funky Turbolaser mailbox access...don't update
        // result register (see stq_c in decoder.isa)
        req->setExtraData(2);
    } else {
        // standard store conditional
        bool lock_flag = xc->readMiscRegNoEffect(LLFlag, tid);
        Addr lock_addr = xc->readMiscRegNoEffect(LLAddr, tid);

        if (!lock_flag || (req->getPaddr() & ~0xf) != lock_addr) {
            // Lock flag not set or addr mismatch in CPU;
            // don't even bother sending to memory system
            req->setExtraData(0);
            xc->setMiscRegNoEffect(LLFlag, false, tid);

            // the rest of this code is not architectural;
            // it's just a debugging aid to help detect
            // livelock by warning on long sequences of failed
            // store conditionals
            int stCondFailures = xc->readStCondFailures();
            stCondFailures++;
            xc->setStCondFailures(stCondFailures);
            if (stCondFailures % 10 == 0) {
                warn("%i: cpu %d: %d consecutive "
                     "store conditional failures\n",
                     curTick, xc->readCpuId(), stCondFailures);
            }

            if (stCondFailures == 5000) {
                panic("Max (5000) Store Conditional Fails Reached. Check Code For Deadlock.\n");
            }

            if (!lock_flag){
                DPRINTF(LLSC, "[tid:%i]: Lock Flag Set, Store Conditional Failed.\n",
                        tid);
            } else if ((req->getPaddr() & ~0xf) != lock_addr) {
                DPRINTF(LLSC, "[tid:%i]: Load-Link Address Mismatch, Store Conditional Failed.\n",
                        tid);
            }
            // store conditional failed already, so don't issue it to mem
            return false;
        }
    }

    return true;
}


} // namespace MipsISA

#endif
