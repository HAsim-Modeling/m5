import m5
from m5.objects import *
m5.AddToPath('../configs/common')
import FSConfig

cpu = AtomicSimpleCPU(cpu_id=0)
system = FSConfig.makeSparcSystem('atomic')
system.cpu = cpu
cpu.connectMemPorts(system.membus)

root = Root(system=system)

m5.ticks.setGlobalFrequency('2GHz')
