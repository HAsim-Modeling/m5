import m5
from m5.objects import *
m5.AddToPath('../configs/common')
from FSConfig import *
from Benchmarks import *

test_sys = makeLinuxAlphaSystem('atomic',
                                 SysConfig('netperf-stream-client.rcS'))
test_sys.cpu = AtomicSimpleCPU(cpu_id=0)
test_sys.cpu.connectMemPorts(test_sys.membus)

drive_sys = makeLinuxAlphaSystem('atomic',
                                 SysConfig('netperf-server.rcS'))
drive_sys.cpu = AtomicSimpleCPU(cpu_id=0)
drive_sys.cpu.connectMemPorts(drive_sys.membus)

root = makeDualRoot(test_sys, drive_sys, "ethertrace")

maxtick = 199999999
