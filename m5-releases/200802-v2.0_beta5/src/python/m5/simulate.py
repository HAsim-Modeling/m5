# Copyright (c) 2005
# The Regents of The University of Michigan
# All Rights Reserved
#
# This code is part of the M5 simulator.
#
# Permission is granted to use, copy, create derivative works and
# redistribute this software and such derivative works for any
# purpose, so long as the copyright notice above, this grant of
# permission, and the disclaimer below appear in all copies made; and
# so long as the name of The University of Michigan is not used in any
# advertising or publicity pertaining to the use or distribution of
# this software without specific, written prior authorization.
#
# THIS SOFTWARE IS PROVIDED AS IS, WITHOUT REPRESENTATION FROM THE
# UNIVERSITY OF MICHIGAN AS TO ITS FITNESS FOR ANY PURPOSE, AND
# WITHOUT WARRANTY BY THE UNIVERSITY OF MICHIGAN OF ANY KIND, EITHER
# EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE. THE REGENTS OF THE UNIVERSITY OF MICHIGAN SHALL NOT BE
# LIABLE FOR ANY DAMAGES, INCLUDING DIRECT, SPECIAL, INDIRECT,
# INCIDENTAL, OR CONSEQUENTIAL DAMAGES, WITH RESPECT TO ANY CLAIM
# ARISING OUT OF OR IN CONNECTION WITH THE USE OF THE SOFTWARE, EVEN
# IF IT HAS BEEN OR IS HEREAFTER ADVISED OF THE POSSIBILITY OF SUCH
# DAMAGES.
#
# Authors: Nathan L. Binkert
#          Steven K. Reinhardt

import atexit
import os
import sys

# import the SWIG-wrapped main C++ functions
import internal
from main import options
import SimObject
import ticks
import objects

# The final hook to generate .ini files.  Called from the user script
# once the config is built.
def instantiate(root):
    # we need to fix the global frequency
    ticks.fixGlobalFrequency()

    root.unproxy_all()

    ini_file = file(os.path.join(options.outdir, 'config.ini'), 'w')
    root.print_ini(ini_file)
    ini_file.close() # close config.ini

    # Initialize the global statistics
    internal.stats.initSimStats()

    # Create the C++ sim objects and connect ports
    root.createCCObject()
    root.connectPorts()

    # Do a second pass to finish initializing the sim objects
    internal.core.initAll()

    # Do a third pass to initialize statistics
    internal.core.regAllStats()

    # Check to make sure that the stats package is properly initialized
    internal.stats.check()

    # Reset to put the stats in a consistent state.
    internal.stats.reset()

def doDot(root):
    dot = pydot.Dot()
    instance.outputDot(dot)
    dot.orientation = "portrait"
    dot.size = "8.5,11"
    dot.ranksep="equally"
    dot.rank="samerank"
    dot.write("config.dot")
    dot.write_ps("config.ps")

need_resume = []
need_startup = True
def simulate(*args, **kwargs):
    global need_resume, need_startup

    if need_startup:
        internal.core.SimStartup()
        need_startup = False

    for root in need_resume:
        resume(root)
    need_resume = []

    return internal.event.simulate(*args, **kwargs)

# Export curTick to user script.
def curTick():
    return internal.core.cvar.curTick

# Python exit handlers happen in reverse order.  We want to dump stats last.
atexit.register(internal.stats.dump)

# register our C++ exit callback function with Python
atexit.register(internal.core.doExitCleanup)

# This loops until all objects have been fully drained.
def doDrain(root):
    all_drained = drain(root)
    while (not all_drained):
        all_drained = drain(root)

# Tries to drain all objects.  Draining might not be completed unless
# all objects return that they are drained on the first call.  This is
# because as objects drain they may cause other objects to no longer
# be drained.
def drain(root):
    all_drained = False
    drain_event = internal.event.createCountedDrain()
    unready_objects = root.startDrain(drain_event, True)
    # If we've got some objects that can't drain immediately, then simulate
    if unready_objects > 0:
        drain_event.setCount(unready_objects)
        simulate()
    else:
        all_drained = True
    internal.event.cleanupCountedDrain(drain_event)
    return all_drained

def resume(root):
    root.resume()

def checkpoint(root, dir):
    if not isinstance(root, objects.Root):
        raise TypeError, "Checkpoint must be called on a root object."
    doDrain(root)
    print "Writing checkpoint"
    internal.core.serializeAll(dir)
    resume(root)

def restoreCheckpoint(root, dir):
    print "Restoring from checkpoint"
    internal.core.unserializeAll(dir)
    need_resume.append(root)

def changeToAtomic(system):
    if not isinstance(system, (objects.Root, objects.System)):
        raise TypeError, "Parameter of type '%s'.  Must be type %s or %s." % \
              (type(system), objects.Root, objects.System)
    if system.getMemoryMode() != objects.params.atomic:
        doDrain(system)
        print "Changing memory mode to atomic"
        system.changeTiming(objects.params.atomic)

def changeToTiming(system):
    if not isinstance(system, (objects.Root, objects.System)):
        raise TypeError, "Parameter of type '%s'.  Must be type %s or %s." % \
              (type(system), objects.Root, objects.System)

    if system.getMemoryMode() != objects.params.timing:
        doDrain(system)
        print "Changing memory mode to timing"
        system.changeTiming(objects.params.timing)

def switchCpus(cpuList):
    print "switching cpus"
    if not isinstance(cpuList, list):
        raise RuntimeError, "Must pass a list to this function"
    for item in cpuList:
        if not isinstance(item, tuple) or len(item) != 2:
            raise RuntimeError, "List must have tuples of (oldCPU,newCPU)"

    for old_cpu, new_cpu in cpuList:
        if not isinstance(old_cpu, objects.BaseCPU):
            raise TypeError, "%s is not of type BaseCPU" % old_cpu
        if not isinstance(new_cpu, objects.BaseCPU):
            raise TypeError, "%s is not of type BaseCPU" % new_cpu

    # Now all of the CPUs are ready to be switched out
    for old_cpu, new_cpu in cpuList:
        old_cpu._ccObject.switchOut()

    for old_cpu, new_cpu in cpuList:
        new_cpu.takeOverFrom(old_cpu)
