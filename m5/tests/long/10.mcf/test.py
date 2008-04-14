m5.AddToPath('../configs/common')
from cpu2000 import mcf

workload = mcf(isa, opsys, 'smred')
root.system.cpu.workload = workload.makeLiveProcess()
root.system.physmem.range=AddrRange('256MB')
