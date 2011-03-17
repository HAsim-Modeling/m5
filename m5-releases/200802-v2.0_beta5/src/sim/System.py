from m5.SimObject import SimObject
from m5.params import *
from m5.proxy import *
from m5 import build_env
from PhysicalMemory import *

class MemoryMode(Enum): vals = ['invalid', 'atomic', 'timing']

class System(SimObject):
    type = 'System'
    swig_objdecls = [ '%include "python/swig/system.i"' ]

    physmem = Param.PhysicalMemory(Parent.any, "phsyical memory")
    mem_mode = Param.MemoryMode('atomic', "The mode the memory system is in")
    if build_env['FULL_SYSTEM']:
        abstract = True
        boot_cpu_frequency = Param.Frequency(Self.cpu[0].clock.frequency,
                                             "boot processor frequency")
        init_param = Param.UInt64(0, "numerical value to pass into simulator")
        boot_osflags = Param.String("a", "boot flags to pass to the kernel")
        kernel = Param.String("", "file that contains the kernel code")
        readfile = Param.String("", "file to read startup script from")
        symbolfile = Param.String("", "file to get the symbols from")
