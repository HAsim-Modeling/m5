# Copyright (c) 2003, 2004, 2005, 2006
# The Regents of The University of Michigan
# All Rights Reserved
#
# This code is part of the M5 simulator.
#
# Permission is granted to use, copy, create derivative works and
# redistribute this software and such derivative works for any
# purpose, so long as the copyright notice above, this grant of
# permission, and the disclaimer below appear in all copies made; and
# so long as the name of The University of Michigan is not used in any
# advertising or publicity pertaining to the use or distribution of
# this software without specific, written prior authorization.
#
# THIS SOFTWARE IS PROVIDED AS IS, WITHOUT REPRESENTATION FROM THE
# UNIVERSITY OF MICHIGAN AS TO ITS FITNESS FOR ANY PURPOSE, AND
# WITHOUT WARRANTY BY THE UNIVERSITY OF MICHIGAN OF ANY KIND, EITHER
# EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE. THE REGENTS OF THE UNIVERSITY OF MICHIGAN SHALL NOT BE
# LIABLE FOR ANY DAMAGES, INCLUDING DIRECT, SPECIAL, INDIRECT,
# INCIDENTAL, OR CONSEQUENTIAL DAMAGES, WITH RESPECT TO ANY CLAIM
# ARISING OUT OF OR IN CONNECTION WITH THE USE OF THE SOFTWARE, EVEN
# IF IT HAS BEEN OR IS HEREAFTER ADVISED OF THE POSSIBILITY OF SUCH
# DAMAGES.
#
# Authors: Steven K. Reinhardt

import os
import os.path
import sys

################
# CpuModel class
#
# The CpuModel class encapsulates everything the ISA parser needs to
# know about a particular CPU model.

class CpuModel:
    # Dict of available CPU model objects.  Accessible as CpuModel.dict.
    dict = {}

    # Constructor.  Automatically adds models to CpuModel.dict.
    def __init__(self, name, filename, includes, strings):
        self.name = name
        self.filename = filename   # filename for output exec code
        self.includes = includes   # include files needed in exec file
        # The 'strings' dict holds all the per-CPU symbols we can
        # substitute into templates etc.
        self.strings = strings
        # Add self to dict
        CpuModel.dict[name] = self

#
# Define CPU models.
#
# Parameters are:
#   - name of model
#   - filename for generated ISA execution file
#   - includes needed for generated ISA execution file
#   - substitution strings for ISA description templates
#

CpuModel('AtomicSimpleCPU', 'atomic_simple_cpu_exec.cc',
         '#include "cpu/simple/atomic.hh"',
         { 'CPU_exec_context': 'AtomicSimpleCPU' })
CpuModel('TimingSimpleCPU', 'timing_simple_cpu_exec.cc',
         '#include "cpu/simple/timing.hh"',
         { 'CPU_exec_context': 'TimingSimpleCPU' })
CpuModel('FullCPU', 'full_cpu_exec.cc',
         '#include "encumbered/cpu/full/dyn_inst.hh"',
         { 'CPU_exec_context': 'DynInst' })
CpuModel('OzoneSimpleCPU', 'ozone_simple_exec.cc',
         '#include "cpu/ozone/dyn_inst.hh"',
         { 'CPU_exec_context': 'OzoneDynInst<SimpleImpl>' })
CpuModel('OzoneCPU', 'ozone_exec.cc',
         '#include "cpu/ozone/dyn_inst.hh"',
         { 'CPU_exec_context': 'OzoneDynInst<OzoneImpl>' })
CpuModel('CheckerCPU', 'checker_cpu_exec.cc',
         '#include "cpu/checker/cpu.hh"',
         { 'CPU_exec_context': 'CheckerCPU' })
CpuModel('O3CPU', 'o3_cpu_exec.cc',
         '#include "cpu/o3/isa_specific.hh"',
         { 'CPU_exec_context': 'O3DynInst' })
