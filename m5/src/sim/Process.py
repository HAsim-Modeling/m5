from m5.SimObject import SimObject
from m5.params import *
from m5.proxy import *

class Process(SimObject):
    type = 'Process'
    abstract = True
    input = Param.String('cin', "filename for stdin")
    output = Param.String('cout', 'filename for stdout/stderr')
    system = Param.System(Parent.any, "system process will run on")
    max_stack_size = Param.MemorySize('64MB', 'maximum size of the stack')

class LiveProcess(Process):
    type = 'LiveProcess'
    executable = Param.String('', "executable (overrides cmd[0] if set)")
    cmd = VectorParam.String("command line (executable plus arguments)")
    env = VectorParam.String([], "environment settings")
    cwd = Param.String('', "current working directory")
    uid = Param.Int(100, 'user id')
    euid = Param.Int(100, 'effective user id')
    gid = Param.Int(100, 'group id')
    egid = Param.Int(100, 'effective group id')
    pid = Param.Int(100, 'process id')
    ppid = Param.Int(99, 'parent process id')
    simpoint = Param.UInt64(0, 'simulation point at which to start simulation')
