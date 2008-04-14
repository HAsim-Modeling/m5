m5.AddToPath('../configs/common')
from cpu2000 import twolf
import os

workload = twolf(isa, opsys, 'smred')
root.system.cpu.workload = workload.makeLiveProcess()
cwd = root.system.cpu.workload[0].cwd

#Remove two files who's presence or absence affects execution
sav_file = os.path.join(cwd, workload.input_set + '.sav')
sv2_file = os.path.join(cwd, workload.input_set + '.sv2')
try:
    os.unlink(sav_file)
except:
    print "Couldn't unlink ", sav_file
try:
    os.unlink(sv2_file)
except:
    print "Couldn't unlink ", sv2_file
