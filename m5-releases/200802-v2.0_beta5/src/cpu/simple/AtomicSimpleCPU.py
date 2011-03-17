from m5.params import *
from m5 import build_env
from BaseCPU import BaseCPU

class AtomicSimpleCPU(BaseCPU):
    type = 'AtomicSimpleCPU'
    width = Param.Int(1, "CPU width")
    simulate_stalls = Param.Bool(False, "Simulate cache stall cycles")
    function_trace = Param.Bool(False, "Enable function trace")
    function_trace_start = Param.Tick(0, "Cycle to start function trace")
    if build_env['FULL_SYSTEM']:
        profile = Param.Latency('0ns', "trace the kernel stack")
    icache_port = Port("Instruction Port")
    dcache_port = Port("Data Port")
    physmem_port = Port("Physical Memory Port")
    _mem_ports = BaseCPU._mem_ports + \
                    ['icache_port', 'dcache_port', 'physmem_port']
