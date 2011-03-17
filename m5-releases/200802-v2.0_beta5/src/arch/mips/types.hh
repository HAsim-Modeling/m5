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

#ifndef __ARCH_MIPS_TYPES_HH__
#define __ARCH_MIPS_TYPES_HH__

#include "sim/host.hh"

namespace MipsISA
{
    typedef uint32_t MachInst;
    typedef uint64_t ExtMachInst;
    typedef uint16_t  RegIndex;

    typedef uint32_t IntReg;
    typedef uint64_t LargestRead;


    // floating point register file entry type
    typedef uint32_t FloatReg32;
    typedef uint64_t FloatReg64;
    typedef uint64_t FloatRegBits;

    typedef double FloatRegVal;
    typedef double FloatReg;

    // cop-0/cop-1 system control register
    typedef uint64_t MiscReg;

    typedef union {
        IntReg   intreg;
        FloatReg fpreg;
        MiscReg  ctrlreg;
    } AnyReg;

    typedef int RegContextParam;
    typedef int RegContextVal;

    //used in FP convert & round function
    enum ConvertType{
        SINGLE_TO_DOUBLE,
        SINGLE_TO_WORD,
        SINGLE_TO_LONG,

        DOUBLE_TO_SINGLE,
        DOUBLE_TO_WORD,
        DOUBLE_TO_LONG,

        LONG_TO_SINGLE,
        LONG_TO_DOUBLE,
        LONG_TO_WORD,
        LONG_TO_PS,

        WORD_TO_SINGLE,
        WORD_TO_DOUBLE,
        WORD_TO_LONG,
        WORD_TO_PS,

        PL_TO_SINGLE,
        PU_TO_SINGLE
    };

    //used in FP convert & round function
    enum RoundMode{
        RND_ZERO,
        RND_DOWN,
        RND_UP,
        RND_NEAREST
   };

struct CoreSpecific {
      /* Note: It looks like it will be better to allow simulator users
         to specify the values of individual variables instead of requiring
         users to define the values of entire registers
         Especially since a lot of these variables can be created from other
         user parameters  (cache descriptions)
                                               -jpp
      */
      // MIPS CP0 State - First individual variables
      // Page numbers refer to revision 2.50 (July 2005) of the MIPS32 ARM, Volume III (PRA)
      unsigned CP0_IntCtl_IPTI; // Page 93, IP Timer Interrupt
      unsigned CP0_IntCtl_IPPCI; // Page 94, IP Performance Counter Interrupt
      unsigned CP0_SrsCtl_HSS; // Page 95, Highest Implemented Shadow Set
      unsigned CP0_PRId_CompanyOptions; // Page 105, Manufacture options
      unsigned CP0_PRId_CompanyID; // Page 105, Company ID - (0-255, 1=>MIPS)
      unsigned CP0_PRId_ProcessorID; // Page 105
      unsigned CP0_PRId_Revision; // Page 105
      unsigned CP0_EBase_CPUNum; // Page 106, CPU Number in a multiprocessor system
      unsigned CP0_Config_BE; // Page 108, Big/Little Endian mode
      unsigned CP0_Config_AT; //Page 109
      unsigned CP0_Config_AR; //Page 109
      unsigned CP0_Config_MT; //Page 109
      unsigned CP0_Config_VI; //Page 109
      unsigned CP0_Config1_M; // Page 110
      unsigned CP0_Config1_MMU; // Page 110
      unsigned CP0_Config1_IS; // Page 110
      unsigned CP0_Config1_IL; // Page 111
      unsigned CP0_Config1_IA; // Page 111
      unsigned CP0_Config1_DS; // Page 111
      unsigned CP0_Config1_DL; // Page 112
      unsigned CP0_Config1_DA; // Page 112
      bool CP0_Config1_C2; // Page 112
      bool CP0_Config1_MD;// Page 112 - Technically not used in MIPS32
      bool CP0_Config1_PC;// Page 112
      bool CP0_Config1_WR;// Page 113
      bool CP0_Config1_CA;// Page 113
      bool CP0_Config1_EP;// Page 113
      bool CP0_Config1_FP;// Page 113
      bool CP0_Config2_M; // Page 114
      unsigned CP0_Config2_TU;// Page 114
      unsigned CP0_Config2_TS;// Page 114
      unsigned CP0_Config2_TL;// Page 115
      unsigned CP0_Config2_TA;// Page 115
      unsigned CP0_Config2_SU;// Page 115
      unsigned CP0_Config2_SS;// Page 115
      unsigned CP0_Config2_SL;// Page 116
      unsigned CP0_Config2_SA;// Page 116
      bool CP0_Config3_M; //// Page 117
      bool CP0_Config3_DSPP;// Page 117
      bool CP0_Config3_LPA;// Page 117
      bool CP0_Config3_VEIC;// Page 118
      bool CP0_Config3_VInt; // Page 118
      bool CP0_Config3_SP;// Page 118
      bool CP0_Config3_MT;// Page 119
      bool CP0_Config3_SM;// Page 119
      bool CP0_Config3_TL;// Page 119

      bool CP0_WatchHi_M; // Page 124
      bool CP0_PerfCtr_M; // Page 130
      bool CP0_PerfCtr_W; // Page 130


      // Then, whole registers
      unsigned CP0_PRId;
      unsigned CP0_Config;
      unsigned CP0_Config1;
      unsigned CP0_Config2;
      unsigned CP0_Config3;
};

} // namespace MipsISA

#endif
