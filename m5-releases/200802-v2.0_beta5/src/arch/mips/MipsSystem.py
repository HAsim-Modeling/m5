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
#


from m5.SimObject import SimObject
from m5.params import *
from m5.proxy import *
from m5 import build_env
from System import System

class MipsSystem(System):
    type = 'MipsSystem'
    console = Param.String("file that contains the console code")
    bare_iron = Param.Bool(False, "Using Bare Iron Mode?")
    hex_file_name = Param.String("test.hex","hex file that contains [address,data] pairs")
    system_type = Param.UInt64("Type of system we are emulating")
    system_rev = Param.UInt64("Revision of system we are emulating")

if build_env['FULL_SYSTEM']:
    class LinuxMipsSystem(MipsSystem):
        type = 'LinuxMipsSystem'
        system_type = 34
        system_rev = 1 << 10

    class BareIronMipsSystem(MipsSystem):
        type = 'BareIronMipsSystem'
	bare_iron = True
        system_type = 34
        system_rev = 1 << 10
        hex_file_name = Param.String('test.hex',"hex file that contains [address,data] pairs")

