from m5.params import *
from m5.proxy import *
from Device import BasicPioDevice

class MipsConsole(BasicPioDevice):
    type = 'MipsConsole'
    cpu = Param.BaseCPU(Parent.cpu[0], "Processor")
    disk = Param.SimpleDisk("Simple Disk")
    sim_console = Param.SimConsole(Parent.any, "The Simulator Console")
    system = Param.MipsSystem(Parent.any, "system object")
