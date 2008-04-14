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
 * Authors: Gabe M. Black
 */

#ifndef __ARCH_SPARC_UTILITY_HH__
#define __ARCH_SPARC_UTILITY_HH__

#include "arch/sparc/faults.hh"
#include "arch/sparc/isa_traits.hh"
#include "arch/sparc/tlb.hh"
#include "base/misc.hh"
#include "base/bitfield.hh"
#include "cpu/thread_context.hh"

namespace SparcISA
{

    
    uint64_t getArgument(ThreadContext *tc, int number, bool fp);

    static inline bool
    inUserMode(ThreadContext *tc)
    {
        return !(tc->readMiscRegNoEffect(MISCREG_PSTATE & (1 << 2)) ||
                tc->readMiscRegNoEffect(MISCREG_HPSTATE & (1 << 2)));
    }

    inline bool isCallerSaveIntegerRegister(unsigned int reg) {
        panic("register classification not implemented");
        return false;
    }

    inline bool isCalleeSaveIntegerRegister(unsigned int reg) {
        panic("register classification not implemented");
        return false;
    }

    inline bool isCallerSaveFloatRegister(unsigned int reg) {
        panic("register classification not implemented");
        return false;
    }

    inline bool isCalleeSaveFloatRegister(unsigned int reg) {
        panic("register classification not implemented");
        return false;
    }

    // Instruction address compression hooks
    inline Addr realPCToFetchPC(const Addr &addr)
    {
        return addr;
    }

    inline Addr fetchPCToRealPC(const Addr &addr)
    {
        return addr;
    }

    // the size of "fetched" instructions (not necessarily the size
    // of real instructions for PISA)
    inline size_t fetchInstSize()
    {
        return sizeof(MachInst);
    }

    /**
     * Function to insure ISA semantics about 0 registers.
     * @param tc The thread context.
     */
    template <class TC>
    void zeroRegisters(TC *tc);

    inline void initCPU(ThreadContext *tc, int cpuId)
    {
        static Fault por = new PowerOnReset();
        if (cpuId == 0)
            por->invoke(tc);

    }

    inline void startupCPU(ThreadContext *tc, int cpuId)
    {
#if FULL_SYSTEM
        // Other CPUs will get activated by IPIs
        if (cpuId == 0)
            tc->activate(0);
#else
        tc->activate(0);
#endif
    }

} // namespace SparcISA

#endif
