import m5
from m5.objects import *

# --------------------
# Base L1 Cache
# ====================

class L1(BaseCache):
    latency = '1ns'
    block_size = 64
    mshrs = 12
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

#MAX CORES IS 8 with the fals sharing method
nb_cores = 8
cpus = [ MemTest() for i in xrange(nb_cores) ]

# system simulated
system = System(cpu = cpus, funcmem = PhysicalMemory(),
                physmem = PhysicalMemory(),
                membus = Bus(clock="500GHz", width=16))

# l2cache & bus
system.toL2Bus = Bus(clock="500GHz", width=16)
system.l2c = L2(size='64kB', assoc=8)
system.l2c.cpu_side = system.toL2Bus.port

# connect l2c to membus
system.l2c.mem_side = system.membus.port

# add L1 caches
for cpu in cpus:
    cpu.l1c = L1(size = '32kB', assoc = 4)
    cpu.l1c.cpu_side = cpu.test
    cpu.l1c.mem_side = system.toL2Bus.port
    system.funcmem.port = cpu.functional

# connect memory to membus
system.physmem.port = system.membus.port


# -----------------------
# run simulation
# -----------------------

root = Root( system = system )
root.system.mem_mode = 'timing'
#root.trace.flags="Cache CachePort MemoryAccess"
#root.trace.cycle=1

