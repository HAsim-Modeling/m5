from m5.SimObject import SimObject
from m5.params import *
from InstTracer import InstTracer

class NativeTrace(InstTracer):
    type = 'NativeTrace'
    cxx_namespace = 'Trace'
    cxx_class = 'NativeTrace'
