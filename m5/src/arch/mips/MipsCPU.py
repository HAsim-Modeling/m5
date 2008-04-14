# -*- mode:python -*-

# Copyright (c) 2007 MIPS Technologies, Inc.  All Rights Reserved
#
# This software is part of the M5 simulator.
#
# THIS IS A LEGAL AGREEMENT.  BY DOWNLOADING, USING, COPYING, CREATING
# DERIVATIVE WORKS, AND/OR DISTRIBUTING THIS SOFTWARE YOU ARE AGREEING
# TO THESE TERMS AND CONDITIONS.
#
# Permission is granted to use, copy, create derivative works and
# distribute this software and such derivative works for any purpose,
# so long as (1) the copyright notice above, this grant of permission,
# and the disclaimer below appear in all copies and derivative works
# made, (2) the copyright notice above is augmented as appropriate to
# reflect the addition of any new copyrightable work in a derivative
# work (e.g., Copyright (c) <Publication Year> Copyright Owner), and (3)
# the name of MIPS Technologies, Inc. ($(B!H(BMIPS$(B!I(B) is not used in any
# advertising or publicity pertaining to the use or distribution of
# this software without specific, written prior authorization.
#
# THIS SOFTWARE IS PROVIDED $(B!H(BAS IS.$(B!I(B  MIPS MAKES NO WARRANTIES AND
# DISCLAIMS ALL WARRANTIES, WHETHER EXPRESS, STATUTORY, IMPLIED OR
# OTHERWISE, INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND
# NON-INFRINGEMENT OF THIRD PARTY RIGHTS, REGARDING THIS SOFTWARE.
# IN NO EVENT SHALL MIPS BE LIABLE FOR ANY DAMAGES, INCLUDING DIRECT,
# INDIRECT, INCIDENTAL, CONSEQUENTIAL, SPECIAL, OR PUNITIVE DAMAGES OF
# ANY KIND OR NATURE, ARISING OUT OF OR IN CONNECTION WITH THIS AGREEMENT,
# THIS SOFTWARE AND/OR THE USE OF THIS SOFTWARE, WHETHER SUCH LIABILITY
# IS ASSERTED ON THE BASIS OF CONTRACT, TORT (INCLUDING NEGLIGENCE OR
# STRICT LIABILITY), OR OTHERWISE, EVEN IF MIPS HAS BEEN WARNED OF THE
# POSSIBILITY OF ANY SUCH LOSS OR DAMAGE IN ADVANCE.
#
# Authors: Jaidev Patwardhan
#          Korey L. Sewell

from m5.SimObject import SimObject
from m5.params import *
from BaseCPU import BaseCPU

class BaseMipsCPU(BaseCPU)
    if build_env['TARGET_ISA'] == 'mips':
        CP0_IntCtl_IPTI = Param.Unsigned(0,"No Description")
        CP0_IntCtl_IPPCI = Param.Unsigned(0,"No Description")
        CP0_SrsCtl_HSS = Param.Unsigned(0,"No Description")
        CP0_EBase_CPUNum = Param.Unsigned(0,"No Description")
        CP0_PRId_CompanyOptions = Param.Unsigned(0,"Company Options in Processor ID Register")
        CP0_PRId_CompanyID = Param.Unsigned(0,"Company Identifier in Processor ID Register")
        CP0_PRId_ProcessorID = Param.Unsigned(1,"Processor ID (0=>Not MIPS32/64 Processor, 1=>MIPS, 2-255 => Other Company")
        CP0_PRId_Revision = Param.Unsigned(0,"Processor Revision Number in Processor ID Register")
        CP0_Config_BE = Param.Unsigned(0,"Big Endian?")
        CP0_Config_AT = Param.Unsigned(0,"No Description")
        CP0_Config_AR = Param.Unsigned(0,"No Description")
        CP0_Config_MT = Param.Unsigned(0,"No Description")
        CP0_Config_VI = Param.Unsigned(0,"No Description")
        CP0_Config1_M = Param.Unsigned(0,"Config2 Implemented?")
        CP0_Config1_MMU = Param.Unsigned(0,"MMU Type")
        CP0_Config1_IS = Param.Unsigned(0,"No Description")
        CP0_Config1_IL = Param.Unsigned(0,"No Description")
        CP0_Config1_IA = Param.Unsigned(0,"No Description")
        CP0_Config1_DS = Param.Unsigned(0,"No Description")
        CP0_Config1_DL = Param.Unsigned(0,"No Description")
        CP0_Config1_DA = Param.Unsigned(0,"No Description")
        CP0_Config1_C2 = Param.Bool(False,"No Description")
        CP0_Config1_MD = Param.Bool(False,"No Description")
        CP0_Config1_PC = Param.Bool(False,"No Description")
        CP0_Config1_WR = Param.Bool(False,"No Description")
        CP0_Config1_CA = Param.Bool(False,"No Description")
        CP0_Config1_EP = Param.Bool(False,"No Description")
        CP0_Config1_FP = Param.Bool(False,"FPU Implemented?")
        CP0_Config2_M = Param.Bool(False,"Config3 Implemented?")
        CP0_Config2_TU = Param.Unsigned(0,"No Description")
        CP0_Config2_TS = Param.Unsigned(0,"No Description")
        CP0_Config2_TL = Param.Unsigned(0,"No Description")
        CP0_Config2_TA = Param.Unsigned(0,"No Description")
        CP0_Config2_SU = Param.Unsigned(0,"No Description")
        CP0_Config2_SS = Param.Unsigned(0,"No Description")
        CP0_Config2_SL = Param.Unsigned(0,"No Description")
        CP0_Config2_SA = Param.Unsigned(0,"No Description")
        CP0_Config3_M = Param.Bool(False,"Config4 Implemented?")
        CP0_Config3_DSPP = Param.Bool(False,"DSP Extensions Present?")
        CP0_Config3_LPA = Param.Bool(False,"No Description")
        CP0_Config3_VEIC = Param.Bool(False,"No Description")
        CP0_Config3_VInt = Param.Bool(False,"No Description")
        CP0_Config3_SP = Param.Bool(False,"No Description")
        CP0_Config3_MT = Param.Bool(False,"Multithreading Extensions Present?")
        CP0_Config3_SM = Param.Bool(False,"No Description")
        CP0_Config3_TL = Param.Bool(False,"No Description")
        CP0_WatchHi_M = Param.Bool(False,"No Description")
        CP0_PerfCtr_M = Param.Bool(False,"No Description")
        CP0_PerfCtr_W = Param.Bool(False,"No Description")
        CP0_PRId = Param.Unsigned(0,"CP0 Status Register")
        CP0_Config = Param.Unsigned(0,"CP0 Config Register")
        CP0_Config1 = Param.Unsigned(0,"CP0 Config1 Register")
        CP0_Config2 = Param.Unsigned(0,"CP0 Config2 Register")
        CP0_Config3 = Param.Unsigned(0,"CP0 Config3 Register")
