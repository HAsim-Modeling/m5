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
 *          Ali G. Sadi
 */

#ifndef __ARCH_SPARC_ISA_TRAITS_HH__
#define __ARCH_SPARC_ISA_TRAITS_HH__

#include "arch/sparc/types.hh"
#include "arch/sparc/max_inst_regs.hh"
#include "arch/sparc/sparc_traits.hh"
#include "config/full_system.hh"
#include "sim/host.hh"

class StaticInstPtr;

namespace BigEndianGuest {}

namespace SparcISA
{
    class RegFile;

    const int MachineBytes = 8;

    //This makes sure the big endian versions of certain functions are used.
    using namespace BigEndianGuest;
    using SparcISAInst::MaxInstSrcRegs;
    using SparcISAInst::MaxInstDestRegs;

    // SPARC has a delay slot
    #define ISA_HAS_DELAY_SLOT 1

    // SPARC NOP (sethi %(hi(0), g0)
    const MachInst NoopMachInst = 0x01000000;

    // These enumerate all the registers for dependence tracking.
    enum DependenceTags {
        FP_Base_DepTag = 32*3+9,
        Ctrl_Base_DepTag = FP_Base_DepTag + 64
    };

    // semantically meaningful register indices
    const int ZeroReg = 0;	// architecturally meaningful
    // the rest of these depend on the ABI
    const int StackPointerReg = 14;
    const int ReturnAddressReg = 31; // post call, precall is 15
    const int ReturnValueReg = 8; // Post return, 24 is pre-return.
    const int FramePointerReg = 30;

    const int ArgumentReg[] = {8, 9, 10, 11, 12, 13};
    const int NumArgumentRegs = sizeof(ArgumentReg) / sizeof(const int);

    // Some OS syscall use a second register (o1) to return a second value
    const int SyscallPseudoReturnReg = ArgumentReg[1];

    //8K. This value is implmentation specific; and should probably
    //be somewhere else.
    const int LogVMPageSize = 13;
    const int VMPageSize = (1 << LogVMPageSize);

    // real address virtual mapping
    // sort of like alpha super page, but less frequently used
    const Addr SegKPMEnd  = ULL(0xfffffffc00000000);
    const Addr SegKPMBase = ULL(0xfffffac000000000);

    //Why does both the previous set of constants and this one exist?
    const int PageShift = 13;
    const int PageBytes = 1ULL << PageShift;

    const int BranchPredAddrShiftAmt = 2;

    StaticInstPtr decodeInst(ExtMachInst);

    /////////// TLB Stuff ////////////
    const Addr StartVAddrHole = ULL(0x0000800000000000);
    const Addr EndVAddrHole = ULL(0xFFFF7FFFFFFFFFFF);
    const Addr VAddrAMask = ULL(0xFFFFFFFF);
    const Addr PAddrImplMask = ULL(0x000000FFFFFFFFFF);
    const Addr BytesInPageMask = ULL(0x1FFF);

#if FULL_SYSTEM
    // I don't know what it's for, so I don't
    // know what SPARC's value should be
    // For loading... XXX This maybe could be USegEnd?? --ali
    const Addr LoadAddrMask = ULL(0xffffffffff);

    enum InterruptTypes
    {
        IT_TRAP_LEVEL_ZERO,
        IT_HINTP,
        IT_INT_VEC,
        IT_CPU_MONDO,
        IT_DEV_MONDO,
        IT_RES_ERROR,
        IT_SOFT_INT,
        NumInterruptTypes
    };

#endif
}

#endif // __ARCH_SPARC_ISA_TRAITS_HH__
