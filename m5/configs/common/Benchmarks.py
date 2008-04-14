from SysPaths import *

class SysConfig:
    def __init__(self, script=None, mem=None, disk=None):
        self.scriptname = script
        self.diskname = disk
        self.memsize = mem

    def script(self):
        if self.scriptname:
            return script(self.scriptname)
        else:
            return ''

    def mem(self):
        if self.memsize:
            return self.memsize
        else:
            return '128MB'

    def disk(self):
        if self.diskname:
            return disk(self.diskname)
        else:
            return env.get('LINUX_IMAGE', disk('linux-latest.img'))

# Benchmarks are defined as a key in a dict which is a list of SysConfigs
# The first defined machine is the test system, the others are driving systems

Benchmarks = {
    'PovrayBench':  [SysConfig('povray-bench.rcS', '512MB', 'povray.img')],
    'PovrayAutumn': [SysConfig('povray-autumn.rcS', '512MB', 'povray.img')],

    'NetperfStream':	[SysConfig('netperf-stream-client.rcS'),
                         SysConfig('netperf-server.rcS')],
    'NetperfStreamUdp':	[SysConfig('netperf-stream-udp-client.rcS'),
                         SysConfig('netperf-server.rcS')],
    'NetperfUdpLocal':	[SysConfig('netperf-stream-udp-local.rcS')],
    'NetperfStreamNT':	[SysConfig('netperf-stream-nt-client.rcS'),
                         SysConfig('netperf-server.rcS')],
    'NetperfMaerts':	[SysConfig('netperf-maerts-client.rcS'),
                         SysConfig('netperf-server.rcS')],
    'SurgeStandard':	[SysConfig('surge-server.rcS', '512MB'),
                         SysConfig('surge-client.rcS', '256MB')],
    'SurgeSpecweb':	[SysConfig('spec-surge-server.rcS', '512MB'),
                         SysConfig('spec-surge-client.rcS', '256MB')],
    'Nhfsstone':	[SysConfig('nfs-server-nhfsstone.rcS', '512MB'),
                         SysConfig('nfs-client-nhfsstone.rcS')],
    'Nfs':		[SysConfig('nfs-server.rcS', '900MB'),
                         SysConfig('nfs-client-dbench.rcS')],
    'NfsTcp':		[SysConfig('nfs-server.rcS', '900MB'),
                         SysConfig('nfs-client-tcp.rcS')],
    'IScsiInitiator':	[SysConfig('iscsi-client.rcS', '512MB'),
                         SysConfig('iscsi-server.rcS', '512MB')],
    'IScsiTarget':	[SysConfig('iscsi-server.rcS', '512MB'),
                         SysConfig('iscsi-client.rcS', '512MB')],
    'Validation':	[SysConfig('iscsi-server.rcS', '512MB'),
                         SysConfig('iscsi-client.rcS', '512MB')],
    'Ping':		[SysConfig('ping-server.rcS',),
                         SysConfig('ping-client.rcS')],

    'ValAccDelay':	[SysConfig('devtime.rcS', '512MB')],
    'ValAccDelay2':	[SysConfig('devtimewmr.rcS', '512MB')],
    'ValMemLat':	[SysConfig('micro_memlat.rcS', '512MB')],
    'ValMemLat2MB':	[SysConfig('micro_memlat2mb.rcS', '512MB')],
    'ValMemLat8MB':	[SysConfig('micro_memlat8mb.rcS', '512MB')],
    'ValMemLat':	[SysConfig('micro_memlat8.rcS', '512MB')],
    'ValTlbLat':	[SysConfig('micro_tlblat.rcS', '512MB')],
    'ValSysLat':	[SysConfig('micro_syscall.rcS', '512MB')],
    'ValCtxLat':	[SysConfig('micro_ctx.rcS', '512MB')],
    'ValStream':	[SysConfig('micro_stream.rcS', '512MB')],
    'ValStreamScale':	[SysConfig('micro_streamscale.rcS', '512MB')],
    'ValStreamCopy':	[SysConfig('micro_streamcopy.rcS', '512MB')],

    'MutexTest':        [SysConfig('mutex-test.rcS', '128MB')],

    'bnAn': [SysConfig('/z/saidi/work/m5.newmem.head/configs/boot/bn-app.rcS',
                       '128MB', '/z/saidi/work/bottleneck/bnimg.img')]
}

benchs = Benchmarks.keys()
benchs.sort()
DefinedBenchmarks = ", ".join(benchs)
