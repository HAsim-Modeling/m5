from m5.SimObject import SimObject
from m5.params import *
class AlphaTLB(SimObject):
    type = 'AlphaTLB'
    abstract = True
    size = Param.Int("TLB size")

class AlphaDTB(AlphaTLB):
    type = 'AlphaDTB'
    cxx_namespace = 'AlphaISA'
    cxx_class = 'DTB'

    size = 64

class AlphaITB(AlphaTLB):
    type = 'AlphaITB'
    cxx_namespace = 'AlphaISA'
    cxx_class = 'ITB'

    size = 48
