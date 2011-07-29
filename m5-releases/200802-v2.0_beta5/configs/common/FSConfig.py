import m5
from m5 import makeList
from m5.objects import *
from Benchmarks import *

class CowIdeDisk(IdeDisk):
    image = CowDiskImage(child=RawDiskImage(read_only=True),
                         read_only=False)

    def childImage(self, ci):
        self.image.child.image_file = ci

def makeLinuxAlphaSystem(mem_mode, mdesc = None):
    class BaseTsunami(Tsunami):
        ethernet = NSGigE(pci_bus=0, pci_dev=1, pci_func=0)
        ide = IdeController(disks=[Parent.disk0, Parent.disk2],
                            pci_func=0, pci_dev=0, pci_bus=0)

    self = LinuxAlphaSystem()
    if not mdesc:
        # generic system
        mdesc = SysConfig()
    self.readfile = mdesc.script()
    self.iobus = Bus(bus_id=0)
    self.membus = Bus(bus_id=1)
    self.bridge = Bridge(delay='50ns', nack_delay='4ns')
    self.physmem = PhysicalMemory(range = AddrRange(mdesc.mem()))
    self.bridge.side_a = self.iobus.port
    self.bridge.side_b = self.membus.port
    self.physmem.port = self.membus.port
    self.disk0 = CowIdeDisk(driveID='master')
    self.disk2 = CowIdeDisk(driveID='master')
    self.disk0.childImage(mdesc.disk())
    self.disk2.childImage(disk('linux-bigswap2.img'))
    self.tsunami = BaseTsunami()
    self.tsunami.attachIO(self.iobus)
    self.tsunami.ide.pio = self.iobus.port
    self.tsunami.ethernet.pio = self.iobus.port
    self.simple_disk = SimpleDisk(disk=RawDiskImage(image_file = mdesc.disk(),
                                               read_only = True))
    self.intrctrl = IntrControl()
    self.mem_mode = mem_mode
    self.sim_console = SimConsole()
    self.kernel = binary('vmlinux')
    self.pal = binary('ts_osfpal')
    self.console = binary('console')
    self.boot_osflags = 'root=/dev/hda1 console=ttyS0'

    return self

def makeSparcSystem(mem_mode, mdesc = None):
    class CowMmDisk(MmDisk):
        image = CowDiskImage(child=RawDiskImage(read_only=True),
                             read_only=False)

        def childImage(self, ci):
            self.image.child.image_file = ci

    self = SparcSystem()
    if not mdesc:
        # generic system
        mdesc = SysConfig()
    self.readfile = mdesc.script()
    self.iobus = Bus(bus_id=0)
    self.membus = Bus(bus_id=1)
    self.bridge = Bridge(delay='50ns', nack_delay='4ns')
    self.t1000 = T1000()
    self.t1000.attachOnChipIO(self.membus)
    self.t1000.attachIO(self.iobus)
    self.physmem = PhysicalMemory(range = AddrRange(Addr('1MB'), size = '64MB'), zero = True)
    self.physmem2 = PhysicalMemory(range = AddrRange(Addr('2GB'), size ='256MB'), zero = True)
    self.bridge.side_a = self.iobus.port
    self.bridge.side_b = self.membus.port
    self.physmem.port = self.membus.port
    self.physmem2.port = self.membus.port
    self.rom.port = self.membus.port
    self.nvram.port = self.membus.port
    self.hypervisor_desc.port = self.membus.port
    self.partition_desc.port = self.membus.port
    self.intrctrl = IntrControl()
    self.disk0 = CowMmDisk()
    self.disk0.childImage(disk('disk.s10hw2'))
    self.disk0.pio = self.iobus.port
    self.reset_bin = binary('reset_new.bin')
    self.hypervisor_bin = binary('q_new.bin')
    self.openboot_bin = binary('openboot_new.bin')
    self.nvram_bin = binary('nvram1')
    self.hypervisor_desc_bin = binary('1up-hv.bin')
    self.partition_desc_bin = binary('1up-md.bin')

    return self

def makeLinuxMipsSystem(mem_mode, mdesc = None):
    class BaseMalta(Malta):
        ethernet = NSGigE(pci_bus=0, pci_dev=1, pci_func=0)
        ide = IdeController(disks=[Parent.disk0, Parent.disk2],
                            pci_func=0, pci_dev=0, pci_bus=0)

    self = LinuxMipsSystem()
    if not mdesc:
        # generic system
        mdesc = SysConfig()
    self.readfile = mdesc.script()
    self.iobus = Bus(bus_id=0)
    self.membus = Bus(bus_id=1)
    self.bridge = Bridge(delay='50ns', nack_delay='4ns')
    self.physmem = PhysicalMemory(range = AddrRange('1GB'))
    self.bridge.side_a = self.iobus.port
    self.bridge.side_b = self.membus.port
    self.physmem.port = self.membus.port
    self.disk0 = CowIdeDisk(driveID='master')
    self.disk2 = CowIdeDisk(driveID='master')
    self.disk0.childImage(mdesc.disk())
    self.disk2.childImage(disk('linux-bigswap2.img'))
    self.malta = BaseMalta()
    self.malta.attachIO(self.iobus)
    self.malta.ide.pio = self.iobus.port
    self.malta.ethernet.pio = self.iobus.port
    self.simple_disk = SimpleDisk(disk=RawDiskImage(image_file = mdesc.disk(),
                                               read_only = True))
    self.intrctrl = IntrControl()
    self.mem_mode = mem_mode
    self.sim_console = SimConsole()
    self.kernel = binary('mips/vmlinux')
    self.console = binary('mips/console')
    self.boot_osflags = 'root=/dev/hda1 console=ttyS0'

    return self

def makeDualRoot(testSystem, driveSystem, dumpfile):
    self = Root()
    self.testsys = testSystem
    self.drivesys = driveSystem
    self.etherlink = EtherLink()
    self.etherlink.int0 = Parent.testsys.tsunami.ethernet.interface
    self.etherlink.int1 = Parent.drivesys.tsunami.ethernet.interface

    if dumpfile:
        self.etherdump = EtherDump(file=dumpfile)
        self.etherlink.dump = Parent.etherdump

    return self

def setMipsOptions(TestCPUClass):
        #CP0 Configuration
        TestCPUClass.CoreParams.CP0_PRId_CompanyOptions = 0
        TestCPUClass.CoreParams.CP0_PRId_CompanyID = 1
        TestCPUClass.CoreParams.CP0_PRId_ProcessorID = 147
        TestCPUClass.CoreParams.CP0_PRId_Revision = 0

        #CP0 Interrupt Control
        TestCPUClass.CoreParams.CP0_IntCtl_IPTI = 7
        TestCPUClass.CoreParams.CP0_IntCtl_IPPCI = 7

        # Config Register
        #TestCPUClass.CoreParams.CP0_Config_K23 = 0 # Since TLB
        #TestCPUClass.CoreParams.CP0_Config_KU = 0 # Since TLB
        TestCPUClass.CoreParams.CP0_Config_BE = 0 # Little Endian
        TestCPUClass.CoreParams.CP0_Config_AR = 1 # Architecture Revision 2
        TestCPUClass.CoreParams.CP0_Config_AT = 0 # MIPS32
        TestCPUClass.CoreParams.CP0_Config_MT = 1 # TLB MMU
        #TestCPUClass.CoreParams.CP0_Config_K0 = 2 # Uncached

        #Config 1 Register
        TestCPUClass.CoreParams.CP0_Config1_M = 1 # Config2 Implemented
        TestCPUClass.CoreParams.CP0_Config1_MMU = 63 # TLB Size
        # ***VERY IMPORTANT***
        # Remember to modify CP0_Config1 according to cache specs
        # Examine file ../common/Cache.py
        TestCPUClass.CoreParams.CP0_Config1_IS = 1 # I-Cache Sets Per Way, 16KB cache, i.e., 1 (128)
        TestCPUClass.CoreParams.CP0_Config1_IL = 5 # I-Cache Line Size, default in Cache.py is 64, i.e 5
        TestCPUClass.CoreParams.CP0_Config1_IA = 1 # I-Cache Associativity, default in Cache.py is 2, i.e, a value of 1
        TestCPUClass.CoreParams.CP0_Config1_DS = 2 # D-Cache Sets Per Way (see below), 32KB cache, i.e., 2
        TestCPUClass.CoreParams.CP0_Config1_DL = 5 # D-Cache Line Size, default is 64, i.e., 5
        TestCPUClass.CoreParams.CP0_Config1_DA = 1 # D-Cache Associativity, default is 2, i.e. 1
        TestCPUClass.CoreParams.CP0_Config1_C2 = 0 # Coprocessor 2 not implemented(?)
        TestCPUClass.CoreParams.CP0_Config1_MD = 0 # MDMX ASE not implemented in Mips32
        TestCPUClass.CoreParams.CP0_Config1_PC = 1 # Performance Counters Implemented
        TestCPUClass.CoreParams.CP0_Config1_WR = 0 # Watch Registers Implemented
        TestCPUClass.CoreParams.CP0_Config1_CA = 0 # Mips16e NOT implemented
        TestCPUClass.CoreParams.CP0_Config1_EP = 0 # EJTag Not Implemented
        TestCPUClass.CoreParams.CP0_Config1_FP = 0 # FPU Implemented

        #Config 2 Register
        TestCPUClass.CoreParams.CP0_Config2_M = 1 # Config3 Implemented
        TestCPUClass.CoreParams.CP0_Config2_TU = 0 # Tertiary Cache Control
        TestCPUClass.CoreParams.CP0_Config2_TS = 0 # Tertiary Cache Sets Per Way
        TestCPUClass.CoreParams.CP0_Config2_TL = 0 # Tertiary Cache Line Size
        TestCPUClass.CoreParams.CP0_Config2_TA = 0 # Tertiary Cache Associativity
        TestCPUClass.CoreParams.CP0_Config2_SU = 0 # Secondary Cache Control
        TestCPUClass.CoreParams.CP0_Config2_SS = 0 # Secondary Cache Sets Per Way
        TestCPUClass.CoreParams.CP0_Config2_SL = 0 # Secondary Cache Line Size
        TestCPUClass.CoreParams.CP0_Config2_SA = 0 # Secondary Cache Associativity


        #Config 3 Register
        TestCPUClass.CoreParams.CP0_Config3_M = 0 # Config4 Not Implemented
        TestCPUClass.CoreParams.CP0_Config3_DSPP = 1 # DSP ASE Present
        TestCPUClass.CoreParams.CP0_Config3_LPA = 0 # Large Physical Addresses Not supported in Mips32
        TestCPUClass.CoreParams.CP0_Config3_VEIC = 0 # EIC Supported
        TestCPUClass.CoreParams.CP0_Config3_VInt = 0 # Vectored Interrupts Implemented
        TestCPUClass.CoreParams.CP0_Config3_SP = 0 # Small Pages Supported (PageGrain reg. exists)
        TestCPUClass.CoreParams.CP0_Config3_MT = 0 # MT Not present
        TestCPUClass.CoreParams.CP0_Config3_SM = 0 # SmartMIPS ASE Not implemented
        TestCPUClass.CoreParams.CP0_Config3_TL = 0 # TraceLogic Not implemented

        #SRS Ctl - HSS
        TestCPUClass.CoreParams.CP0_SrsCtl_HSS = 3 # Four shadow register sets implemented


        #TestCPUClass.CoreParams.tlb = TLB()
        #TestCPUClass.CoreParams.UnifiedTLB = 1