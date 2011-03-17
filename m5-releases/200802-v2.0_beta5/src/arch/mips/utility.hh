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
 * Authors: Korey L. Sewell
 */

#ifndef __ARCH_MIPS_UTILITY_HH__
#define __ARCH_MIPS_UTILITY_HH__
#include "config/full_system.hh"
#include "arch/mips/types.hh"
#include "arch/mips/isa_traits.hh"
#include "base/misc.hh"
#include "config/full_system.hh"
//XXX This is needed for size_t. We should use something other than size_t
//#include "kern/linux/linux.hh"
#include "sim/host.hh"

#include "cpu/thread_context.hh"

class ThreadContext;

namespace MipsISA {

    uint64_t getArgument(ThreadContext *tc, int number, bool fp);

    ////////////////////////////////////////////////////////////////////////
    //
    // Floating Point Utility Functions
    //
    uint64_t fpConvert(ConvertType cvt_type, double fp_val);
    double roundFP(double val, int digits);
    double truncFP(double val);

    bool getCondCode(uint32_t fcsr, int cc);
    uint32_t genCCVector(uint32_t fcsr, int num, uint32_t cc_val);
    uint32_t genInvalidVector(uint32_t fcsr);

    bool isNan(void *val_ptr, int size);
    bool isQnan(void *val_ptr, int size);
    bool isSnan(void *val_ptr, int size);

    static inline bool
    inUserMode(ThreadContext *tc)
    {
        MiscReg Stat = tc->readMiscReg(MipsISA::Status);
        MiscReg Dbg = tc->readMiscReg(MipsISA::Debug);

        if((Stat & 0x10000006) == 0  // EXL, ERL or CU0 set, CP0 accessible
           && (Dbg & 0x40000000) == 0 // DM bit set, CP0 accessible
           && (Stat & 0x00000018) != 0) {  // KSU = 0, kernel mode is base mode
            // Unable to use Status_CU0, etc directly, using bitfields & masks
            return true;
        } else {
            return false;
        }
    }

    // Instruction address compression hooks
    static inline Addr realPCToFetchPC(const Addr &addr) {
        return addr;
    }

    static inline Addr fetchPCToRealPC(const Addr &addr) {
        return addr;
    }

    // the size of "fetched" instructions (not necessarily the size
    // of real instructions for PISA)
    static inline size_t fetchInstSize() {
        return sizeof(MachInst);
    }

    ////////////////////////////////////////////////////////////////////////
    //
    //  Register File Utility Functions
    //
    static inline int flattenFloatIndex(ThreadContext * tc, int reg)
    {
        return reg;
    }

    static inline int flattenIntIndex(ThreadContext * tc, int reg)
    {
        // Implement Shadow Sets Stuff Here;
        return reg;
    }

    static inline MachInst makeRegisterCopy(int dest, int src) {
        panic("makeRegisterCopy not implemented");
        return 0;
    }

    void copyRegs(ThreadContext *src, ThreadContext *dest);

    void copyMiscRegs(ThreadContext *src, ThreadContext *dest);


    template <class CPU>
    void zeroRegisters(CPU *cpu);

    ////////////////////////////////////////////////////////////////////////
    //
    //  Translation stuff
    //
    inline Addr
    TruncPage(Addr addr)
    { return addr & ~(PageBytes - 1); }

    inline Addr
    RoundPage(Addr addr)
    { return (addr + PageBytes - 1) & ~(PageBytes - 1); }

    ////////////////////////////////////////////////////////////////////////
    //
    // CPU Utility
    //
    void startupCPU(ThreadContext *tc, int cpuId);
};


#endif
