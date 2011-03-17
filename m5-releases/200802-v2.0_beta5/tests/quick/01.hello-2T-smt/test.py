process1 = LiveProcess(cmd = 'hello', executable = binpath('hello'))
process2 = LiveProcess(cmd = 'hello', executable = binpath('hello'))

root.system.cpu.workload = [process1, process2]
