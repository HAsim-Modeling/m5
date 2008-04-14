/*
 * Copyright (c) 2003, 2004, 2005
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
 *          Ali G. Saidi
 */


#include "arch/alpha/utility.hh"

#if FULL_SYSTEM
#include "arch/alpha/vtophys.hh"
#include "mem/vport.hh"
#endif

namespace AlphaISA
{

uint64_t getArgument(ThreadContext *tc, int number, bool fp)
{
#if FULL_SYSTEM
    if (number < NumArgumentRegs) {
        if (fp)
            return tc->readFloatRegBits(ArgumentReg[number]);
        else
            return tc->readIntReg(ArgumentReg[number]);
    } else {
        Addr sp = tc->readIntReg(StackPointerReg);
        VirtualPort *vp = tc->getVirtPort(tc);
        uint64_t arg = vp->read<uint64_t>(sp + 
                           (number-NumArgumentRegs) * sizeof(uint64_t));
        tc->delVirtPort(vp);
        return arg;
    }
#else
    panic("getArgument() is Full system only\n");
    M5_DUMMY_RETURN
#endif
}

} // namespace AlphaISA

