from m5.SimObject import SimObject
from m5.params import *

class InstTracer(SimObject):
    type = 'InstTracer'
    cxx_namespace = 'Trace'
    abstract = True
