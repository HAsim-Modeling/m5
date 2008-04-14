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
 * the name of MIPS Technologies, Inc. ($(B!H(BMIPS$(B!I(B) is not used in any
 * advertising or publicity pertaining to the use or distribution of
 * this software without specific, written prior authorization.
 *
 * THIS SOFTWARE IS PROVIDED $(B!H(BAS IS.$(B!I(B  MIPS MAKES NO WARRANTIES AND
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

#ifndef __ARCH_MIPS_DT_CONSTANTS_HH__
#define __ARCH_MIPS_DT_CONSTANTS_HH__

#include "arch/mips/types.hh"

namespace MipsISA
{
  // See the EJTAG Specification - Revision 4.10
  // Also see PDTrace Specification - Revision 4.30

  // Debug Register - CP0 Reg 23, Sel 0
  const unsigned Debug_DBD = 31;
  const unsigned Debug_DM_HI = 30;
  const unsigned Debug_DM_LO = 30;
  const unsigned Debug_NODCR = 29;
  const unsigned Debug_LSNM = 28;
  const unsigned Debug_DOZE = 27;
  const unsigned Debug_HALT = 26;
  const unsigned Debug_COUNTDM = 25;
  const unsigned Debug_IBUSEP = 24;
  const unsigned Debug_MCHECKEP = 23;
  const unsigned Debug_CACHEEP = 22;
  const unsigned Debug_DBUSEP = 21;
  const unsigned Debug_IEXI_HI = 20;
  const unsigned Debug_IEXI_LO = 20;
  const unsigned Debug_DDBS_IMPR = 19;
  const unsigned Debug_DDBL_IMPR = 18;
  const unsigned Debug_EJTAGVER_2 =17;
  const unsigned Debug_EJTAGVER_1 =16;
  const unsigned Debug_EJTAGVER_0 =15;
  const unsigned Debug_EJTAGVER_HI = 17;
  const unsigned Debug_EJTAGVER_LO = 15;
  const unsigned Debug_DEXC_CODE_HI = 14;
  const unsigned Debug_DEXC_CODE_LO = 10;
  const unsigned Debug_NOSST = 9;
  const unsigned Debug_SST = 8;
  const unsigned Debug_OFFLINE = 7;
  const unsigned Debug_DIBIMPR = 6;
  const unsigned Debug_DINT = 5;
  const unsigned Debug_DIB = 4;
  const unsigned Debug_DDBS = 3;
  const unsigned Debug_DDBL = 2;
  const unsigned Debug_DBp = 1;
  const unsigned Debug_DSS = 0;


  // TraceControl Register - CP0 Reg 23, Sel 1
  const unsigned TraceControl_TS = 31;
  const unsigned TraceControl_UT = 30;
  const unsigned TraceControl_TB = 27;
  const unsigned TraceControl_IO = 26;
  const unsigned TraceControl_D = 25;
  const unsigned TraceControl_E = 24;
  const unsigned TraceControl_K = 23;
  const unsigned TraceControl_S = 22;
  const unsigned TraceControl_U = 21;
  const unsigned TraceControl_ASID_M_HI = 20;
  const unsigned TraceControl_ASID_M_LO = 13;
  const unsigned TraceControl_ASID_HI = 12;
  const unsigned TraceControl_ASID_LO = 5;
  const unsigned TraceControl_G = 4;
  const unsigned TraceControl_TFCR = 3;
  const unsigned TraceControl_TLSM = 2;
  const unsigned TraceControl_TIM = 1;
  const unsigned TraceControl_ON = 0;

  // TraceControl2 Register - CP0 Reg 23, Sel 2
  const unsigned TraceControl2_CPUIDV = 29;
  const unsigned TraceControl2_CPUID_HI = 28;
  const unsigned TraceControl2_CPUID_LO = 21;
  const unsigned TraceControl2_TCV = 20;
  const unsigned TraceControl2_TCNUM_HI = 19;
  const unsigned TraceControl2_TCNUM_LO = 12;
  const unsigned TraceControl2_MODE_HI = 11;
  const unsigned TraceControl2_MODE_LO = 7;
  const unsigned TraceControl2_VALIDMODES_HI = 6;
  const unsigned TraceControl2_VALIDMODES_LO = 5;
  const unsigned TraceControl2_TBI = 4;
  const unsigned TraceControl2_TBU = 3;
  const unsigned TraceControl2_SYP_HI = 2;
  const unsigned TraceControl2_SYP_LO = 0;

  // UserTraceData Register - CP0 Reg 23, Sel 3
  // Just holds 32-bits (or 64-bits) of data

  // TraceIBPC Register - CP0 Reg 23, Sel 4
  const unsigned TraceIBPC_MB = 31;
  const unsigned TraceIBPC_IE = 28;
  const unsigned TraceIBPC_ATE = 27;
  const unsigned TraceIBPC_IBPC8_HI = 26;
  const unsigned TraceIBPC_IBPC8_LO = 24;
  const unsigned TraceIBPC_IBPC7_HI = 23;
  const unsigned TraceIBPC_IBPC7_LO = 21;
  const unsigned TraceIBPC_IBPC6_HI = 20;
  const unsigned TraceIBPC_IBPC6_LO = 18;
  const unsigned TraceIBPC_IBPC5_HI = 17;
  const unsigned TraceIBPC_IBPC5_LO = 15;
  const unsigned TraceIBPC_IBPC4_HI = 14;
  const unsigned TraceIBPC_IBPC4_LO = 12;
  const unsigned TraceIBPC_IBPC3_HI = 11;
  const unsigned TraceIBPC_IBPC3_LO = 9;
  const unsigned TraceIBPC_IBPC2_HI = 8;
  const unsigned TraceIBPC_IBPC2_LO = 6;
  const unsigned TraceIBPC_IBPC1_HI = 5;
  const unsigned TraceIBPC_IBPC1_LO = 3;
  const unsigned TraceIBPC_IBPC0_HI = 2;
  const unsigned TraceIBPC_IBPC0_LO = 0;


  // TraceDBPC Register - CP0 Reg 23, Sel 5
  const unsigned TRACEDBPC_MB = 31;
  const unsigned TRACEDBPC_DE = 28;
  const unsigned TRACEDBPC_ATE = 27;
  const unsigned TRACEDBPC_DBPC8_HI = 26;
  const unsigned TRACEDBPC_DBPC8_LO = 24;
  const unsigned TRACEDBPC_DBPC7_HI = 23;
  const unsigned TRACEDBPC_DBPC7_LO = 21;
  const unsigned TRACEDBPC_DBPC6_HI = 20;
  const unsigned TRACEDBPC_DBPC6_LO = 18;
  const unsigned TRACEDBPC_DBPC5_HI = 17;
  const unsigned TRACEDBPC_DBPC5_LO = 15;
  const unsigned TRACEDBPC_DBPC4_HI = 14;
  const unsigned TRACEDBPC_DBPC4_LO = 12;
  const unsigned TRACEDBPC_DBPC3_HI = 11;
  const unsigned TRACEDBPC_DBPC3_LO = 9;
  const unsigned TRACEDBPC_DBPC2_HI = 8;
  const unsigned TRACEDBPC_DBPC2_LO = 6;
  const unsigned TRACEDBPC_DBPC1_HI = 5;
  const unsigned TRACEDBPC_DBPC1_LO = 3;
  const unsigned TRACEDBPC_DBPC0_HI = 2;
  const unsigned TRACEDBPC_DBPC0_LO = 0;

  // TraceIBPC2 - Not part of CP0, but part of TRACE
  const unsigned TraceIBPC_IBPC14_HI = 17;
  const unsigned TraceIBPC_IBPC14_LO = 15;
  const unsigned TraceIBPC_IBPC13_HI = 14;
  const unsigned TraceIBPC_IBPC13_LO = 12;
  const unsigned TraceIBPC_IBPC12_HI = 11;
  const unsigned TraceIBPC_IBPC12_LO = 9;
  const unsigned TraceIBPC_IBPC11_HI = 8;
  const unsigned TraceIBPC_IBPC11_LO = 6;
  const unsigned TraceIBPC_IBPC10_HI = 5;
  const unsigned TraceIBPC_IBPC10_LO = 3;
  const unsigned TraceIBPC_IBPC9_HI = 2;
  const unsigned TraceIBPC_IBPC9_LO = 0;


  // TraceDBPC2 - Not part of CP0, but part of TRACE
  const unsigned TRACEDBPC_DBPC14_HI = 17;
  const unsigned TRACEDBPC_DBPC14_LO = 15;
  const unsigned TRACEDBPC_DBPC13_HI = 14;
  const unsigned TRACEDBPC_DBPC13_LO = 12;
  const unsigned TRACEDBPC_DBPC12_HI = 11;
  const unsigned TRACEDBPC_DBPC12_LO = 9;
  const unsigned TRACEDBPC_DBPC11_HI = 8;
  const unsigned TRACEDBPC_DBPC11_LO = 6;
  const unsigned TRACEDBPC_DBPC10_HI = 5;
  const unsigned TRACEDBPC_DBPC10_LO = 3;
  const unsigned TRACEDBPC_DBPC9_HI = 2;
  const unsigned TRACEDBPC_DBPC9_LO = 0;


  // Debug Register 2 - CP0 Reg 23, Sel 6
  const unsigned DEBUG2_PRM = 3;
  const unsigned DEBUG2_DQ = 2;
  const unsigned DEBUG2_TUP = 1;
  const unsigned DEBUG2_PACO = 0;

  // DEPC Register - CP0 Reg 24, Sel 0
  // Debug Exception Program Counter
  const unsigned DEPC_HI = 31;
  const unsigned DEPC_LO = 0;



  // DESAVE - CP0 Reg 31, Sel 0
  // Debug Exception Save Register
  const unsigned DESAVE_HI = 31;
  const unsigned DESAVE_LO = 0;



} // namespace MipsISA

#endif
