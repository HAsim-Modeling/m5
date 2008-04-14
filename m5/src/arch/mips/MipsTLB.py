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
# the name of MIPS Technologies, Inc. (“MIPS”) is not used in any
# advertising or publicity pertaining to the use or distribution of
# this software without specific, written prior authorization.
#
# THIS SOFTWARE IS PROVIDED “AS IS.”  MIPS MAKES NO WARRANTIES AND
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
#          Korey Sewell

from m5.SimObject import SimObject
from m5.params import *

class MipsTLB(SimObject):
    abstract = True
    type = 'MipsTLB'
    cxx_namespace = 'MipsISA'
    cxx_class = 'TLB'
    size = Param.Int("TLB size")

class MipsDTB(MipsTLB):
    type = 'MipsDTB'
    cxx_namespace = 'MipsISA'
    cxx_class = 'DTB'
    size = 64

class MipsITB(MipsTLB):
    type = 'MipsITB'
    cxx_namespace = 'MipsISA'
    cxx_class = 'ITB'
    size = 64

class MipsUTB(MipsTLB):
    type = 'MipsUTB'
    cxx_namespace = 'MipsISA'
    cxx_class = 'UTB'
    size = 64

