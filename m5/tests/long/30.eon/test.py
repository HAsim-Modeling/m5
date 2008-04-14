m5.AddToPath('../configs/common')
from cpu2000 import eon_cook

workload = eon_cook(isa, opsys, 'mdred')
root.system.cpu.workload = workload.makeLiveProcess()
