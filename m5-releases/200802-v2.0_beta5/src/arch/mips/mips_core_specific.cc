/*
 * Copyright (c) 2007 MIPS Technologies, Inc.  All Rights Reserved
 *
 * This software is part of the M5 simulator.
 *
 * THIS IS A LEGAL AGREEMENT.  BY DOWNLOADING, USING, COPYING, CREATING
 * DERIVATIVE WORKS, AND/OR DISTRIBUTING THIS SOFTWARE YOU ARE AGREEING
 * TO THESE TERMS AND CONDITIONS.
 *
 * Permission is granted to use, copy, create derivative works and
 * distribute this software and such derivative works for any purpose,
 * so long as (1) the copyright notice above, this grant of permission,
 * and the disclaimer below appear in all copies and derivative works
 * made, (2) the copyright notice above is augmented as appropriate to
 * reflect the addition of any new copyrightable work in a derivative
 * work (e.g., Copyright (c) <Publication Year> Copyright Owner), and (3)
 * the name of MIPS Technologies, Inc. (“MIPS”) is not used in any
 * advertising or publicity pertaining to the use or distribution of
 * this software without specific, written prior authorization.
 *
 * THIS SOFTWARE IS PROVIDED “AS IS.”  MIPS MAKES NO WARRANTIES AND
 * DISCLAIMS ALL WARRANTIES, WHETHER EXPRESS, STATUTORY, IMPLIED OR
 * OTHERWISE, INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND
 * NON-INFRINGEMENT OF THIRD PARTY RIGHTS, REGARDING THIS SOFTWARE.
 * IN NO EVENT SHALL MIPS BE LIABLE FOR ANY DAMAGES, INCLUDING DIRECT,
 * INDIRECT, INCIDENTAL, CONSEQUENTIAL, SPECIAL, OR PUNITIVE DAMAGES OF
 * ANY KIND OR NATURE, ARISING OUT OF OR IN CONNECTION WITH THIS AGREEMENT,
 * THIS SOFTWARE AND/OR THE USE OF THIS SOFTWARE, WHETHER SUCH LIABILITY
 * IS ASSERTED ON THE BASIS OF CONTRACT, TORT (INCLUDING NEGLIGENCE OR
 * STRICT LIABILITY), OR OTHERWISE, EVEN IF MIPS HAS BEEN WARNED OF THE
 * POSSIBILITY OF ANY SUCH LOSS OR DAMAGE IN ADVANCE.
 *
 * Authors: Jaidev Patwardhan
 *
 */

#include "arch/mips/faults.hh"
#include "arch/mips/isa_traits.hh"
#include "arch/mips/tlb.hh"
//#include "base/kgdb.h"
#include "base/remote_gdb.hh"
#include "base/stats/events.hh"
#include "config/full_system.hh"
#include "cpu/base.hh"
#include "cpu/simple_thread.hh"
#include "cpu/thread_context.hh"
#include "kern/kernel_stats.hh"
#include "sim/debug.hh"
#include "sim/sim_exit.hh"
#include "arch/mips/mips_core_specific.hh"

#if FULL_SYSTEM

////////////////////////////////////////////////////////////////////////
//
//  Machine dependent functions
//
void
MipsISA::initCPU(ThreadContext *tc, int cpuId)
{

 //    MipsFault *reset = new ResetFault;
//     tc->setPC(reset->vect());
//     tc->setNextPC(tc->readPC() + sizeof(MachInst));

//     delete reset;
}

template <class CPU>
void
MipsISA::processInterrupts(CPU *cpu)
{
    //Check if there are any outstanding interrupts
    //Handle the interrupts
  /*    int ipl = 0;
    int summary = 0;

    cpu->checkInterrupts = false;

    if (cpu->readMiscReg(IPR_ASTRR))
        panic("asynchronous traps not implemented\n");

    if (cpu->readMiscReg(IPR_SIRR)) {
        for (int i = INTLEVEL_SOFTWARE_MIN;
             i < INTLEVEL_SOFTWARE_MAX; i++) {
            if (cpu->readMiscReg(IPR_SIRR) & (ULL(1) << i)) {
                // See table 4-19 of the 21164 hardware reference
                ipl = (i - INTLEVEL_SOFTWARE_MIN) + 1;
                summary |= (ULL(1) << i);
            }
        }
    }

    uint64_t interrupts = cpu->intr_status();

    if (interrupts) {
        for (int i = INTLEVEL_EXTERNAL_MIN;
             i < INTLEVEL_EXTERNAL_MAX; i++) {
            if (interrupts & (ULL(1) << i)) {
                // See table 4-19 of the 21164 hardware reference
                ipl = i;
                summary |= (ULL(1) << i);
            }
        }
    }

    if (ipl && ipl > cpu->readMiscReg(IPR_IPLR)) {
        cpu->setMiscReg(IPR_ISR, summary);
        cpu->setMiscReg(IPR_INTID, ipl);
        cpu->trap(new InterruptFault);
        DPRINTF(Flow, "Interrupt! IPLR=%d ipl=%d summary=%x\n",
                cpu->readMiscReg(IPR_IPLR), ipl, summary);
    }
  */
}


/*int
MipsISA::MiscRegFile::getInstAsid()
{
    return EV5::ITB_ASN_ASN(ipr[IPR_ITB_ASN]);
}

int
MipsISA::MiscRegFile::getDataAsid()
{
    return EV5::DTB_ASN_ASN(ipr[IPR_DTB_ASN]);
    }*/



#endif // FULL_SYSTEM || BARE_IRON
