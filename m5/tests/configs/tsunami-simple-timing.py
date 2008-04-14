import m5
from m5.objects import *
m5.AddToPath('../configs/common')
import FSConfig


# --------------------
# Base L1 Cache
# ====================

class L1(BaseCache):
    latency = '1ns'
    block_size = 64
    mshrs = 4
    tgts_per_mshr = 8

# ----------------------
# Base L2 Cache
# ----------------------

class L2(BaseCache):
    block_size = 64
    latency = '10ns'
    mshrs = 92
    tgts_per_mshr = 16
    write_buffers = 8

# ---------------------
# I/O Cache
# ---------------------
class IOCache(BaseCache):
    assoc = 8
    block_size = 64
    latency = '50ns'
    mshrs = 20
    size = '1kB'
    tgts_per_mshr = 12
    mem_side_filter_ranges=[AddrRange(0, Addr.max)]
    cpu_side_filter_ranges=[AddrRange(0x8000000000, Addr.max)]

#cpu
cpu = TimingSimpleCPU(cpu_id=0)
#the system
system = FSConfig.makeLinuxAlphaSystem('timing')

system.cpu = cpu
#create the l1/l2 bus
system.toL2Bus = Bus()
system.bridge.filter_ranges_a=[AddrRange(0, Addr.max)]
system.bridge.filter_ranges_b=[AddrRange(0, size='8GB')]
system.iocache = IOCache()
system.iocache.cpu_side = system.iobus.port
system.iocache.mem_side = system.membus.port


#connect up the l2 cache
system.l2c = L2(size='4MB', assoc=8)
system.l2c.cpu_side = system.toL2Bus.port
system.l2c.mem_side = system.membus.port

#connect up the cpu and l1s
cpu.addPrivateSplitL1Caches(L1(size = '32kB', assoc = 1),
                            L1(size = '32kB', assoc = 4))
# connect cpu level-1 caches to shared level-2 cache
cpu.connectMemPorts(system.toL2Bus)
cpu.clock = '2GHz'

root = Root(system=system)
m5.ticks.setGlobalFrequency('1THz')

