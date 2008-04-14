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
 *          Steven K. Reinhardt
 */

#ifndef __ARCH_ALPHA_UTILITY_HH__
#define __ARCH_ALPHA_UTILITY_HH__

#include "config/full_system.hh"
#include "arch/alpha/types.hh"
#include "arch/alpha/isa_traits.hh"
#include "arch/alpha/regfile.hh"
#include "base/misc.hh"
#include "cpu/thread_context.hh"

namespace AlphaISA
{

    uint64_t getArgument(ThreadContext *tc, int number, bool fp);
    
    static inline bool
    inUserMode(ThreadContext *tc)
    {
        return (tc->readMiscRegNoEffect(AlphaISA::IPR_DTB_CM) & 0x18) != 0;
    }

    inline bool isCallerSaveIntegerRegister(unsigned int reg) {
        panic("register classification not implemented");
        return (reg >= 1 && reg <= 8 || reg >= 22 && reg <= 25 || reg == 27);
    }

    inline bool isCalleeSaveIntegerRegister(unsigned int reg) {
        panic("register classification not implemented");
        return (reg >= 9 && reg <= 15);
    }

    inline bool isCallerSaveFloatRegister(unsigned int reg) {
        panic("register classification not implemented");
        return false;
    }

    inline bool isCalleeSaveFloatRegister(unsigned int reg) {
        panic("register classification not implemented");
        return false;
    }

    inline Addr alignAddress(const Addr &addr,
                                         unsigned int nbytes) {
        return (addr & ~(nbytes - 1));
    }

    // Instruction address compression hooks
    inline Addr realPCToFetchPC(const Addr &addr) {
        return addr;
    }

    inline Addr fetchPCToRealPC(const Addr &addr) {
        return addr;
    }

    // the size of "fetched" instructions (not necessarily the size
    // of real instructions for PISA)
    inline size_t fetchInstSize() {
        return sizeof(MachInst);
    }

    inline MachInst makeRegisterCopy(int dest, int src) {
        panic("makeRegisterCopy not implemented");
        return 0;
    }

    // Machine operations

    void saveMachineReg(AnyReg &savereg, const RegFile &reg_file,
                               int regnum);

    void restoreMachineReg(RegFile &regs, const AnyReg &reg,
                                  int regnum);

    /**
     * Function to insure ISA semantics about 0 registers.
     * @param tc The thread context.
     */
    template <class TC>
    void zeroRegisters(TC *tc);

    // Alpha IPR register accessors
    inline bool PcPAL(Addr addr) { return addr & 0x3; }
    inline void startupCPU(ThreadContext *tc, int cpuId) {
        tc->activate(0);
    }

    ////////////////////////////////////////////////////////////////////////
    //
    //  Translation stuff
    //

    inline Addr PteAddr(Addr a) { return (a & PteMask) << PteShift; }

    // User Virtual
    inline bool IsUSeg(Addr a) { return USegBase <= a && a <= USegEnd; }

    // Kernel Direct Mapped
    inline bool IsK0Seg(Addr a) { return K0SegBase <= a && a <= K0SegEnd; }
    inline Addr K0Seg2Phys(Addr addr) { return addr & ~K0SegBase; }

    // Kernel Virtual
    inline bool IsK1Seg(Addr a) { return K1SegBase <= a && a <= K1SegEnd; }

    inline Addr
    TruncPage(Addr addr)
    { return addr & ~(PageBytes - 1); }

    inline Addr
    RoundPage(Addr addr)
    { return (addr + PageBytes - 1) & ~(PageBytes - 1); }

    void initIPRs(ThreadContext *tc, int cpuId);
#if FULL_SYSTEM
    void initCPU(ThreadContext *tc, int cpuId);

    /**
     * Function to check for and process any interrupts.
     * @param tc The thread context.
     */
    template <class TC>
    void processInterrupts(TC *tc);
#endif

} // namespace AlphaISA

#endif
