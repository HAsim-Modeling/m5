m5.AddToPath('../configs/common')
from cpu2000 import bzip2_source

workload = bzip2_source(isa, opsys, 'lgred')
root.system.cpu.workload = workload.makeLiveProcess()
