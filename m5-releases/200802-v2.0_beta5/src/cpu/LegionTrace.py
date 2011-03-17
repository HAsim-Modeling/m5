from m5.SimObject import SimObject
from m5.params import *
from InstTracer import InstTracer

class LegionTrace(InstTracer):
    type = 'LegionTrace'
    cxx_namespace = 'Trace'
    cxx_class = 'LegionTrace'
