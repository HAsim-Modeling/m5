import os, sys

# single "path" arg encodes everything we need to know about test
(category, name, isa, opsys, config) = sys.argv[1].split('/')

# find path to directory containing this file
tests_root = os.path.dirname(__file__)
if os.path.isdir('/dist/m5/regression/test-progs'):
    test_progs = '/dist/m5/regression/test-progs'
else:
    test_progs = os.path.join(tests_root, 'test-progs')

# generate path to binary file
def binpath(app, file=None):
    # executable has same name as app unless specified otherwise
    if not file:
        file = app
    return os.path.join(test_progs, app, 'bin', isa, opsys, file)

# generate path to input file
def inputpath(app, file=None):
    # input file has same name as app unless specified otherwise
    if not file:
        file = app
    return os.path.join(test_progs, app, 'input', file)

# build configuration
execfile(os.path.join(tests_root, 'configs', config + '.py'))

# set default maxtick... script can override
# -1 means run forever
from m5 import MaxTick
maxtick = MaxTick

# tweak configuration for specific test

execfile(os.path.join(tests_root, category, name, 'test.py'))

# instantiate configuration
m5.instantiate(root)

# simulate until program terminates
exit_event = m5.simulate(maxtick)

print 'Exiting @ tick', m5.curTick(), 'because', exit_event.getCause()
