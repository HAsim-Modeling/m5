from m5.params import *
from m5.proxy import *
from Device import BasicPioDevice
from Platform import Platform
from MipsConsole import MipsConsole
from Uart import Uart8250
from Pci import PciConfigAll
from BadDevice import BadDevice

class MaltaCChip(BasicPioDevice):
    type = 'MaltaCChip'
    malta = Param.Malta(Parent.any, "Malta")

class MaltaIO(BasicPioDevice):
    type = 'MaltaIO'
    time = Param.UInt64(1136073600,
        "System time to use (0 for actual time, default is 1/1/06)")
    malta = Param.Malta(Parent.any, "Malta")
    frequency = Param.Frequency('1050Hz', "frequency of interrupts")

class MaltaPChip(BasicPioDevice):
    type = 'MaltaPChip'
    malta = Param.Malta(Parent.any, "Malta")

class Malta(Platform):
    type = 'Malta'
    system = Param.System(Parent.any, "system")
    cchip = MaltaCChip(pio_addr=0x801a0000000)
    io = MaltaIO(pio_addr=0x801fc000000)
    uart = Uart8250(pio_addr=0xBFD003F8)
    console = MipsConsole(pio_addr=0xBFD00F00, disk=Parent.simple_disk)

    # Attach I/O devices to specified bus object.  Can't do this
    # earlier, since the bus object itself is typically defined at the
    # System level.
    def attachIO(self, bus):
        self.cchip.pio = bus.port
        self.io.pio = bus.port
        self.uart.pio = bus.port
        self.console.pio = bus.port
