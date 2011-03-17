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
 * Authors: Steven K. Reinhardt
 *          Gabe M. Black
 */

#ifndef __ARCH_ALPHA_ISA_TRAITS_HH__
#define __ARCH_ALPHA_ISA_TRAITS_HH__

namespace LittleEndianGuest {}

#include "arch/alpha/ipr.hh"
#include "arch/alpha/max_inst_regs.hh"
#include "arch/alpha/types.hh"
#include "config/full_system.hh"
#include "sim/host.hh"

class StaticInstPtr;

namespace AlphaISA
{
    using namespace LittleEndianGuest;
    using AlphaISAInst::MaxInstSrcRegs;
    using AlphaISAInst::MaxInstDestRegs;

    // These enumerate all the registers for dependence tracking.
    enum DependenceTags {
        // 0..31 are the integer regs 0..31
        // 32..63 are the FP regs 0..31, i.e. use (reg + FP_Base_DepTag)
        FP_Base_DepTag = 40,
        Ctrl_Base_DepTag = 72
    };

    StaticInstPtr decodeInst(ExtMachInst);

    // Alpha Does NOT have a delay slot
    #define ISA_HAS_DELAY_SLOT 0

    const Addr PageShift = 13;
    const Addr PageBytes = ULL(1) << PageShift;
    const Addr PageMask = ~(PageBytes - 1);
    const Addr PageOffset = PageBytes - 1;


    ////////////////////////////////////////////////////////////////////////
    //
    //  Translation stuff
    //

    const Addr PteShift = 3;
    const Addr NPtePageShift = PageShift - PteShift;
    const Addr NPtePage = ULL(1) << NPtePageShift;
    const Addr PteMask = NPtePage - 1;

    // User Virtual
    const Addr USegBase = ULL(0x0);
    const Addr USegEnd = ULL(0x000003ffffffffff);

    // Kernel Direct Mapped
    const Addr K0SegBase = ULL(0xfffffc0000000000);
    const Addr K0SegEnd = ULL(0xfffffdffffffffff);

    // Kernel Virtual
    const Addr K1SegBase = ULL(0xfffffe0000000000);
    const Addr K1SegEnd = ULL(0xffffffffffffffff);

    // For loading... XXX This maybe could be USegEnd?? --ali
    const Addr LoadAddrMask = ULL(0xffffffffff);

#if FULL_SYSTEM

    ////////////////////////////////////////////////////////////////////////
    //
    //  Interrupt levels
    //
    enum InterruptLevels
    {
        INTLEVEL_SOFTWARE_MIN = 4,
        INTLEVEL_SOFTWARE_MAX = 19,

        INTLEVEL_EXTERNAL_MIN = 20,
        INTLEVEL_EXTERNAL_MAX = 34,

        INTLEVEL_IRQ0 = 20,
        INTLEVEL_IRQ1 = 21,
        INTINDEX_ETHERNET = 0,
        INTINDEX_SCSI = 1,
        INTLEVEL_IRQ2 = 22,
        INTLEVEL_IRQ3 = 23,

        INTLEVEL_SERIAL = 33,

        NumInterruptLevels = INTLEVEL_EXTERNAL_MAX
    };

#endif

    // EV5 modes
    enum mode_type
    {
        mode_kernel = 0,		// kernel
        mode_executive = 1,		// executive (unused by unix)
        mode_supervisor = 2,	// supervisor (unused by unix)
        mode_user = 3,		// user mode
        mode_number			// number of modes
    };

    // Constants Related to the number of registers

    const int NumIntArchRegs = 32;
    const int NumPALShadowRegs = 8;
    const int NumFloatArchRegs = 32;
    // @todo: Figure out what this number really should be.
    const int NumMiscArchRegs = 77;

    const int NumIntRegs = NumIntArchRegs + NumPALShadowRegs;
    const int NumFloatRegs = NumFloatArchRegs;
    const int NumMiscRegs = NumMiscArchRegs;

    const int TotalNumRegs = NumIntRegs + NumFloatRegs +
        NumMiscRegs + NumInternalProcRegs;

    const int TotalDataRegs = NumIntRegs + NumFloatRegs;

    // semantically meaningful register indices
    const int ZeroReg = 31;	// architecturally meaningful
    // the rest of these depend on the ABI
    const int StackPointerReg = 30;
    const int GlobalPointerReg = 29;
    const int ProcedureValueReg = 27;
    const int ReturnAddressReg = 26;
    const int ReturnValueReg = 0;
    const int FramePointerReg = 15;

    const int ArgumentReg[] = {16, 17, 18, 19, 20, 21};
    const int NumArgumentRegs = sizeof(ArgumentReg) / sizeof(const int);

    const int SyscallNumReg = ReturnValueReg;
    const int SyscallPseudoReturnReg = ArgumentReg[4];
    const int SyscallSuccessReg = 19;

    const int LogVMPageSize = 13;	// 8K bytes
    const int VMPageSize = (1 << LogVMPageSize);

    const int BranchPredAddrShiftAmt = 2; // instructions are 4-byte aligned

    const int MachineBytes = 8;
    const int WordBytes = 4;
    const int HalfwordBytes = 2;
    const int ByteBytes = 1;

    // return a no-op instruction... used for instruction fetch faults
    // Alpha UNOP (ldq_u r31,0(r0))
    const ExtMachInst NoopMachInst = 0x2ffe0000;

};

#endif // __ARCH_ALPHA_ISA_TRAITS_HH__
