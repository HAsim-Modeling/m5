m5.AddToPath('../configs/common')
from cpu2000 import parser

workload = parser(isa, opsys, 'mdred')
root.system.cpu.workload = workload.makeLiveProcess()
