# workload
benchmarks = [
    "tests/test-progs/hello/bin/alpha/linux/hello", "'hello'",
    "tests/test-progs/hello/bin/alpha/linux/hello", "'hello'",
    "tests/test-progs/hello/bin/alpha/linux/hello", "'hello'",
    "tests/test-progs/hello/bin/alpha/linux/hello", "'hello'",
    ]

for i, cpu in zip(range(len(cpus)), root.system.cpu):
    p            = LiveProcess()
    p.executable = benchmarks[i*2]
    p.cmd        = benchmarks[(i*2)+1]
    root.system.cpu[i].workload = p
    root.system.cpu[i].max_insts_all_threads = 10000000
#root.system.cpu.workload = LiveProcess(cmd = 'hello',
 #                                      executable = binpath('hello'))
