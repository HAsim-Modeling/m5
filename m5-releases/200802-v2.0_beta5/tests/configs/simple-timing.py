import m5
from m5.objects import *

class MyCache(BaseCache):
    assoc = 2
    block_size = 64
    latency = '1ns'
    mshrs = 10
    tgts_per_mshr = 5

cpu = TimingSimpleCPU(cpu_id=0)
cpu.addTwoLevelCacheHierarchy(MyCache(size = '128kB'), MyCache(size = '256kB'),
                              MyCache(size = '2MB', latency='10ns'))
system = System(cpu = cpu,
                physmem = PhysicalMemory(),
                membus = Bus())
system.physmem.port = system.membus.port
cpu.connectMemPorts(system.membus)
cpu.clock = '2GHz'

root = Root(system = system)
