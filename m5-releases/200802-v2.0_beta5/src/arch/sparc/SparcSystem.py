from m5.params import *

from PhysicalMemory import *
from System import System

class SparcSystem(System):
    type = 'SparcSystem'
    _rom_base = 0xfff0000000
    _nvram_base = 0x1f11000000
    _hypervisor_desc_base = 0x1f12080000
    _partition_desc_base = 0x1f12000000
    # ROM for OBP/Reset/Hypervisor
    rom = Param.PhysicalMemory(
        PhysicalMemory(range=AddrRange(_rom_base, size='8MB')),
            "Memory to hold the ROM data")
    # nvram
    nvram = Param.PhysicalMemory(
        PhysicalMemory(range=AddrRange(_nvram_base, size='8kB')),
        "Memory to hold the nvram data")
    # hypervisor description
    hypervisor_desc = Param.PhysicalMemory(
        PhysicalMemory(range=AddrRange(_hypervisor_desc_base, size='8kB')),
        "Memory to hold the hypervisor description")
    # partition description
    partition_desc = Param.PhysicalMemory(
        PhysicalMemory(range=AddrRange(_partition_desc_base, size='8kB')),
        "Memory to hold the partition description")

    reset_addr = Param.Addr(_rom_base, "Address to load ROM at")
    hypervisor_addr = Param.Addr(Addr('64kB') + _rom_base,
                                 "Address to load hypervisor at")
    openboot_addr = Param.Addr(Addr('512kB') + _rom_base,
                               "Address to load openboot at")
    nvram_addr = Param.Addr(_nvram_base, "Address to put the nvram")
    hypervisor_desc_addr = Param.Addr(_hypervisor_desc_base,
            "Address for the hypervisor description")
    partition_desc_addr = Param.Addr(_partition_desc_base,
            "Address for the partition description")

    reset_bin = Param.String("file that contains the reset code")
    hypervisor_bin = Param.String("file that contains the hypervisor code")
    openboot_bin = Param.String("file that contains the openboot code")
    nvram_bin = Param.String("file that contains the contents of nvram")
    hypervisor_desc_bin = Param.String("file that contains the hypervisor description")
    partition_desc_bin = Param.String("file that contains the partition description")

