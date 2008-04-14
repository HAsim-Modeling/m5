m5.AddToPath('../configs/common')
from cpu2000 import gzip_log

workload = gzip_log(isa, opsys, 'smred')
root.system.cpu.workload = workload.makeLiveProcess()
