import m5
from m5.objects import *
m5.AddToPath('../configs/common')

class MyCache(BaseCache):
    assoc = 2
    block_size = 64
    latency = '1ns'
    mshrs = 10
    tgts_per_mshr = 5

cpu = DerivO3CPU(cpu_id=0)
cpu.addTwoLevelCacheHierarchy(MyCache(size = '128kB'), MyCache(size = '256kB'),
                              MyCache(size = '2MB'))
cpu.clock = '2GHz'

system = System(cpu = cpu,
                physmem = PhysicalMemory(),
                membus = Bus())
system.physmem.port = system.membus.port
cpu.connectMemPorts(system.membus)

root = Root(system = system)
