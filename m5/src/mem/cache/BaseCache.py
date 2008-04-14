from m5.params import *
from m5.proxy import Self
from MemObject import MemObject

class Prefetch(Enum): vals = ['none', 'tagged', 'stride', 'ghb']

class BaseCache(MemObject):
    type = 'BaseCache'
    assoc = Param.Int("associativity")
    block_size = Param.Int("block size in bytes")
    latency = Param.Latency("Latency")
    hash_delay = Param.Int(1, "time in cycles of hash access")
    lifo = Param.Bool(False,
        "whether this NIC partition should use LIFO repl. policy")
    max_miss_count = Param.Counter(0,
        "number of misses to handle before calling exit")
    mshrs = Param.Int("number of MSHRs (max outstanding requests)")
    prioritizeRequests = Param.Bool(False,
        "always service demand misses first")
    repl = Param.Repl(NULL, "replacement policy")
    size = Param.MemorySize("capacity in bytes")
    split = Param.Bool(False, "whether or not this cache is split")
    split_size = Param.Int(0,
        "How many ways of the cache belong to CPU/LRU partition")
    subblock_size = Param.Int(0,
        "Size of subblock in IIC used for compression")
    tgts_per_mshr = Param.Int("max number of accesses per MSHR")
    trace_addr = Param.Addr(0, "address to trace")
    two_queue = Param.Bool(False,
        "whether the lifo should have two queue replacement")
    write_buffers = Param.Int(8, "number of write buffers")
    prefetch_miss = Param.Bool(False,
         "wheter you are using the hardware prefetcher from Miss stream")
    prefetch_access = Param.Bool(False,
         "wheter you are using the hardware prefetcher from Access stream")
    prefetcher_size = Param.Int(100,
         "Number of entries in the harware prefetch queue")
    prefetch_past_page = Param.Bool(False,
         "Allow prefetches to cross virtual page boundaries")
    prefetch_serial_squash = Param.Bool(False,
         "Squash prefetches with a later time on a subsequent miss")
    prefetch_degree = Param.Int(1,
         "Degree of the prefetch depth")
    prefetch_latency = Param.Latency(10 * Self.latency,
         "Latency of the prefetcher")
    prefetch_policy = Param.Prefetch('none',
         "Type of prefetcher to use")
    prefetch_cache_check_push = Param.Bool(True,
         "Check if in cash on push or pop of prefetch queue")
    prefetch_use_cpu_id = Param.Bool(True,
         "Use the CPU ID to seperate calculations of prefetches")
    prefetch_data_accesses_only = Param.Bool(False,
         "Only prefetch on data not on instruction accesses")
    cpu_side = Port("Port on side closer to CPU")
    mem_side = Port("Port on side closer to MEM")
    cpu_side_filter_ranges = VectorParam.AddrRange([],
            "What addresses shouldn't be passed through the side of the bridge")
    mem_side_filter_ranges = VectorParam.AddrRange([],
            "What addresses shouldn't be passed through the side of the bridge")
    addr_range = VectorParam.AddrRange(AllMemory, "The address range in bytes")
