from m5.params import *
from System import System

class AlphaSystem(System):
    type = 'AlphaSystem'
    console = Param.String("file that contains the console code")
    pal = Param.String("file that contains palcode")
    system_type = Param.UInt64("Type of system we are emulating")
    system_rev = Param.UInt64("Revision of system we are emulating")

class LinuxAlphaSystem(AlphaSystem):
    type = 'LinuxAlphaSystem'
    system_type = 34
    system_rev = 1 << 10

class FreebsdAlphaSystem(AlphaSystem):
    type = 'FreebsdAlphaSystem'
    system_type = 34
    system_rev = 1 << 10

class Tru64AlphaSystem(AlphaSystem):
    type = 'Tru64AlphaSystem'
    system_type = 12
    system_rev = 2<<1
