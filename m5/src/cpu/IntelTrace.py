from m5.SimObject import SimObject
from m5.params import *
from InstTracer import InstTracer

class IntelTrace(InstTracer):
    type = 'IntelTrace'
    cxx_namespace = 'Trace'
    cxx_class = 'IntelTrace'
