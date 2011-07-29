from m5.params import *
from m5.proxy import *
from MemObject import *

class PhysicalMemory(MemObject):
    type = 'PhysicalMemory'
    port = VectorPort("the access port")
    range = Param.AddrRange(AddrRange('128MB'), "Device Address")
    file = Param.String('', "memory mapped file")
    latency = Param.Latency('1t', "latency of an access")
    zero = Param.Bool(False, "zero initialize memory")

class DRAMMemory(PhysicalMemory):
    type = 'DRAMMemory'
    # Many of these should be observed from the configuration
    cpu_ratio = Param.Int(5,"ratio between CPU speed and memory bus speed")
    mem_type = Param.String("SDRAM", "Type of DRAM (DRDRAM, SDRAM)")
    mem_actpolicy = Param.String("open", "Open/Close policy")
    memctrladdr_type = Param.String("interleaved", "Mapping interleaved or direct")
    bus_width = Param.Int(16, "")
    act_lat = Param.Latency("2ns", "RAS to CAS delay")
    cas_lat = Param.Latency("1ns", "CAS delay")
    war_lat = Param.Latency("2ns", "write after read delay")
    pre_lat = Param.Latency("2ns", "precharge delay")
    dpl_lat = Param.Latency("2ns", "data in to precharge delay")
    trc_lat = Param.Latency("6ns", "row cycle delay")
    num_banks = Param.Int(4, "Number of Banks")
    num_cpus = Param.Int(4, "Number of CPUs connected to DRAM")
