import m5
from m5.objects import *
m5.AddToPath('../configs/common')

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

nb_cores = 4
cpus = [ DerivO3CPU(cpu_id=i) for i in xrange(nb_cores) ]

# system simulated
system = System(cpu = cpus, physmem = PhysicalMemory(), membus =
Bus())

# l2cache & bus
system.toL2Bus = Bus()
system.l2c = L2(size='4MB', assoc=8)
system.l2c.cpu_side = system.toL2Bus.port

# connect l2c to membus
system.l2c.mem_side = system.membus.port

# add L1 caches
for cpu in cpus:
    cpu.addPrivateSplitL1Caches(L1(size = '32kB', assoc = 1),
                                L1(size = '32kB', assoc = 4))
    # connect cpu level-1 caches to shared level-2 cache
    cpu.connectMemPorts(system.toL2Bus)
    cpu.clock = '2GHz'

# connect memory to membus
system.physmem.port = system.membus.port


# -----------------------
# run simulation
# -----------------------

root = Root( system = system )
root.system.mem_mode = 'timing'
#root.trace.flags="Bus Cache"
#root.trace.flags = "BusAddrRanges"
