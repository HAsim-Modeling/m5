m5.AddToPath('../configs/common')
from cpu2000 import vortex

workload = vortex(isa, opsys, 'smred')
root.system.cpu.workload = workload.makeLiveProcess()
