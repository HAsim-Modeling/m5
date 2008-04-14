from m5.params import *
from m5 import build_env
from BaseCPU import BaseCPU

class O3Checker(BaseCPU):
    type = 'O3Checker'
    exitOnError = Param.Bool(False, "Exit on an error")
    updateOnError = Param.Bool(False,
        "Update the checker with the main CPU's state on an error")
    warnOnlyOnLoadError = Param.Bool(False,
        "If a load result is incorrect, only print a warning and do not exit")
    function_trace = Param.Bool(False, "Enable function trace")
    function_trace_start = Param.Tick(0, "Cycle to start function trace")
    if build_env['FULL_SYSTEM']:
        profile = Param.Latency('0ns', "trace the kernel stack")
