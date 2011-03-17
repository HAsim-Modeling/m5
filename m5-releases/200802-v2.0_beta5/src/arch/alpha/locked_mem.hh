/*
 * Copyright (c) 2006
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

#ifndef __ARCH_ALPHA_LOCKED_MEM_HH__
#define __ARCH_ALPHA_LOCKED_MEM_HH__

/**
 * @file
 *
 * ISA-specific helper functions for locked memory accesses.
 *
 * Note that these functions are not embedded in the ISA description
 * because they operate on the *physical* address rather than the
 * virtual address.  In the current M5 design, the physical address is
 * not accessible from the ISA description, only from the CPU model.
 * Thus the CPU is responsible for calling back to the ISA (here)
 * after the address translation has been performed to allow the ISA
 * to do these manipulations based on the physical address.
 */

#include "arch/alpha/miscregfile.hh"
#include "base/misc.hh"
#include "mem/request.hh"


namespace AlphaISA
{
template <class XC>
inline void
handleLockedRead(XC *xc, Request *req)
{
    xc->setMiscRegNoEffect(MISCREG_LOCKADDR, req->getPaddr() & ~0xf);
    xc->setMiscRegNoEffect(MISCREG_LOCKFLAG, true);
}


template <class XC>
inline bool
handleLockedWrite(XC *xc, Request *req)
{
    if (req->isUncacheable()) {
        // Funky Turbolaser mailbox access...don't update
        // result register (see stq_c in decoder.isa)
        req->setExtraData(2);
    } else {
        // standard store conditional
        bool lock_flag = xc->readMiscRegNoEffect(MISCREG_LOCKFLAG);
        Addr lock_addr = xc->readMiscRegNoEffect(MISCREG_LOCKADDR);
        if (!lock_flag || (req->getPaddr() & ~0xf) != lock_addr) {
            // Lock flag not set or addr mismatch in CPU;
            // don't even bother sending to memory system
            req->setExtraData(0);
            xc->setMiscRegNoEffect(MISCREG_LOCKFLAG, false);
            // the rest of this code is not architectural;
            // it's just a debugging aid to help detect
            // livelock by warning on long sequences of failed
            // store conditionals
            int stCondFailures = xc->readStCondFailures();
            stCondFailures++;
            xc->setStCondFailures(stCondFailures);
            if (stCondFailures % 100000 == 0) {
                warn("cpu %d: %d consecutive "
                     "store conditional failures\n",
                     xc->readCpuId(), stCondFailures);
            }

            // store conditional failed already, so don't issue it to mem
            return false;
        }
    }

    return true;
}


} // namespace AlphaISA

#endif
