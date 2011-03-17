import m5
from m5.objects import *

system = System(cpu = AtomicSimpleCPU(cpu_id=0),
                physmem = PhysicalMemory(),
                membus = Bus())
system.physmem.port = system.membus.port
system.cpu.connectMemPorts(system.membus)
system.cpu.clock = '2GHz'

root = Root(system = system)
