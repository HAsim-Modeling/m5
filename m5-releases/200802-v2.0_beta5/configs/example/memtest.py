import m5
from m5.objects import *
import os, optparse, sys
m5.AddToPath('../common')

parser = optparse.OptionParser()

parser.add_option("-a", "--atomic", action="store_true",
                  help="Use atomic (non-timing) mode")
parser.add_option("-b", "--blocking", action="store_true",
                  help="Use blocking caches")
parser.add_option("-l", "--maxloads", metavar="N", default=0,
                  help="Stop after N loads")
parser.add_option("-m", "--maxtick", type="int", default=m5.MaxTick,
                  metavar="T",
                  help="Stop after T ticks")

#
# The "tree" specification is a colon-separated list of one or more
# integers.  The first integer is the number of caches/testers
# connected directly to main memory.  The last integer in the list is
# the number of testers associated with the uppermost level of memory
# (L1 cache, if there are caches, or main memory if no caches).  Thus
# if there is only one integer, there are no caches, and the integer
# specifies the number of testers connected directly to main memory.
# The other integers (if any) specify the number of caches at each
# level of the hierarchy between.
#
# Examples:
#
#  "2:1"    Two caches connected to memory with a single tester behind each
#           (single-level hierarchy, two testers total)
#
#  "2:2:1"  Two-level hierarchy, 2 L1s behind each of 2 L2s, 4 testers total
#
parser.add_option("-t", "--treespec", type="string", default="8:1",
                  help="Colon-separated multilevel tree specification, "
                  "see script comments for details "
                  "[default: %default]")

parser.add_option("--force-bus", action="store_true",
                  help="Use bus between levels even with single cache")

parser.add_option("-f", "--functional", type="int", default=0,
                  metavar="PCT",
                  help="Target percentage of functional accesses "
                  "[default: %default]")
parser.add_option("-u", "--uncacheable", type="int", default=0,
                  metavar="PCT",
                  help="Target percentage of uncacheable accesses "
                  "[default: %default]")

parser.add_option("--progress", type="int", default=1000,
                  metavar="NLOADS",
                  help="Progress message interval "
                  "[default: %default]")

(options, args) = parser.parse_args()

if args:
     print "Error: script doesn't take any positional arguments"
     sys.exit(1)

block_size = 64

try:
     treespec = [int(x) for x in options.treespec.split(':')]
     numtesters = reduce(lambda x,y: x*y, treespec)
except:
     print "Error parsing treespec option"
     sys.exit(1)

if numtesters > block_size:
     print "Error: Number of testers limited to %s because of false sharing" \
           % (block_size)
     sys.exit(1)

if len(treespec) < 1:
     print "Error parsing treespec"
     sys.exit(1)

# define prototype L1 cache
proto_l1 = BaseCache(size = '32kB', assoc = 4, block_size = block_size,
                     latency = '1ns', tgts_per_mshr = 8)

if options.blocking:
     proto_l1.mshrs = 1
else:
     proto_l1.mshrs = 8

# build a list of prototypes, one for each level of treespec, starting
# at the end (last entry is tester objects)
prototypes = [ MemTest(atomic=options.atomic, max_loads=options.maxloads,
                       percent_functional=options.functional,
                       percent_uncacheable=options.uncacheable,
                       progress_interval=options.progress) ]

# next comes L1 cache, if any
if len(treespec) > 1:
     prototypes.insert(0, proto_l1)

# now add additional cache levels (if any) by scaling L1 params
while len(prototypes) < len(treespec):
     # clone previous level and update params
     prev = prototypes[0]
     next = prev()
     next.size = prev.size * 4
     next.latency = prev.latency * 10
     next.assoc = prev.assoc * 2
     prototypes.insert(0, next)

# system simulated
system = System(funcmem = PhysicalMemory(),
                physmem = PhysicalMemory(latency = "100ns"))

def make_level(spec, prototypes, attach_obj, attach_port):
     fanout = spec[0]
     parent = attach_obj # use attach obj as config parent too
     if len(spec) > 1 and (fanout > 1 or options.force_bus):
          new_bus = Bus(clock="500MHz", width=16)
          new_bus.port = getattr(attach_obj, attach_port)
          parent.cpu_side_bus = new_bus
          attach_obj = new_bus
          attach_port = "port"
     objs = [prototypes[0]() for i in xrange(fanout)]
     if len(spec) > 1:
          # we just built caches, more levels to go
          parent.cache = objs
          for cache in objs:
               cache.mem_side = getattr(attach_obj, attach_port)
               make_level(spec[1:], prototypes[1:], cache, "cpu_side")
     else:
          # we just built the MemTest objects
          parent.cpu = objs
          for t in objs:
               t.test = getattr(attach_obj, attach_port)
               t.functional = system.funcmem.port

make_level(treespec, prototypes, system.physmem, "port")

# -----------------------
# run simulation
# -----------------------

root = Root( system = system )
if options.atomic:
    root.system.mem_mode = 'atomic'
else:
    root.system.mem_mode = 'timing'

# Not much point in this being higher than the L1 latency
m5.ticks.setGlobalFrequency('1ns')

# instantiate configuration
m5.instantiate(root)

# simulate until program terminates
exit_event = m5.simulate(options.maxtick)

print 'Exiting @ tick', m5.curTick(), 'because', exit_event.getCause()