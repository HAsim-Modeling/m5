m5.AddToPath('../configs/common')
from cpu2000 import perlbmk_makerand

workload = perlbmk_makerand(isa, opsys, 'lgred')
root.system.cpu.workload = workload.makeLiveProcess()
