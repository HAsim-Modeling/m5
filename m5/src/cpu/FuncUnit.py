from m5.SimObject import SimObject
from m5.params import *

class OpClass(Enum):
    vals = ['No_OpClass', 'IntAlu', 'IntMult', 'IntDiv', 'FloatAdd',
            'FloatCmp', 'FloatCvt', 'FloatMult', 'FloatDiv', 'FloatSqrt',
            'MemRead', 'MemWrite', 'IprAccess', 'InstPrefetch']

class OpDesc(SimObject):
    type = 'OpDesc'
    issueLat = Param.Int(1, "cycles until another can be issued")
    opClass = Param.OpClass("type of operation")
    opLat = Param.Int(1, "cycles until result is available")

class FUDesc(SimObject):
    type = 'FUDesc'
    count = Param.Int("number of these FU's available")
    opList = VectorParam.OpDesc("operation classes for this FU type")
