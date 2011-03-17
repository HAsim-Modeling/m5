from m5.SimObject import SimObject
from m5.params import *
class SparcTLB(SimObject):
    type = 'SparcTLB'
    abstract = True
    size = Param.Int("TLB size")

class SparcDTB(SparcTLB):
    type = 'SparcDTB'
    cxx_namespace = 'SparcISA'
    cxx_class = 'DTB'

    size = 64

class SparcITB(SparcTLB):
    type = 'SparcITB'
    cxx_namespace = 'SparcISA'
    cxx_class = 'ITB'

    size = 64
