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
#

import code
import datetime
import optparse
import os
import socket
import sys

from attrdict import attrdict
import defines
import traceflags

__all__ = [ 'options', 'arguments', 'main' ]

usage="%prog [m5 options] script.py [script options]"
version="%prog 2.0"
brief_copyright='''
Copyright (c) 2001-2008
The Regents of The University of Michigan
All Rights Reserved
'''

def print_list(items, indent=4):
    line = ' ' * indent
    for i,item in enumerate(items):
        if len(line) + len(item) > 76:
            print line
            line = ' ' * indent

        if i < len(items) - 1:
            line += '%s, ' % item
        else:
            line += item
            print line

# there's only one option parsing done, so make it global and add some
# helper functions to make it work well.
parser = optparse.OptionParser(usage=usage, version=version,
                               description=brief_copyright,
                               formatter=optparse.TitledHelpFormatter())
parser.disable_interspersed_args()

# current option group
group = None

def set_group(*args, **kwargs):
    '''set the current option group'''
    global group
    if not args and not kwargs:
        group = None
    else:
        group = parser.add_option_group(*args, **kwargs)

class splitter(object):
    def __init__(self, split):
        self.split = split
    def __call__(self, option, opt_str, value, parser):
        getattr(parser.values, option.dest).extend(value.split(self.split))

def add_option(*args, **kwargs):
    '''add an option to the current option group, or global none set'''

    # if action=split, but allows the option arguments
    # themselves to be lists separated by the split variable'''

    if kwargs.get('action', None) == 'append' and 'split' in kwargs:
        split = kwargs.pop('split')
        kwargs['default'] = []
        kwargs['type'] = 'string'
        kwargs['action'] = 'callback'
        kwargs['callback'] = splitter(split)

    if group:
        return group.add_option(*args, **kwargs)

    return parser.add_option(*args, **kwargs)

def bool_option(name, default, help):
    '''add a boolean option called --name and --no-name.
    Display help depending on which is the default'''

    tname = '--%s' % name
    fname = '--no-%s' % name
    dest = name.replace('-', '_')
    if default:
        thelp = optparse.SUPPRESS_HELP
        fhelp = help
    else:
        thelp = help
        fhelp = optparse.SUPPRESS_HELP

    add_option(tname, action="store_true", default=default, help=thelp)
    add_option(fname, action="store_false", dest=dest, help=fhelp)

# Help options
add_option('-A', "--authors", action="store_true", default=False,
    help="Show author information")
add_option('-B', "--build-info", action="store_true", default=False,
    help="Show build information")
add_option('-C', "--copyright", action="store_true", default=False,
    help="Show full copyright information")
add_option('-R', "--readme", action="store_true", default=False,
    help="Show the readme")
add_option('-N', "--release-notes", action="store_true", default=False,
    help="Show the release notes")

# Options for configuring the base simulator
add_option('-d', "--outdir", metavar="DIR", default=".",
    help="Set the output directory to DIR [Default: %default]")
add_option('-i', "--interactive", action="store_true", default=False,
    help="Invoke the interactive interpreter after running the script")
add_option("--pdb", action="store_true", default=False,
    help="Invoke the python debugger before running the script")
add_option('-p', "--path", metavar="PATH[:PATH]", action='append', split=':',
    help="Prepend PATH to the system path when invoking the script")
add_option('-q', "--quiet", action="count", default=0,
    help="Reduce verbosity")
add_option('-v', "--verbose", action="count", default=0,
    help="Increase verbosity")

# Statistics options
set_group("Statistics Options")
add_option("--stats-file", metavar="FILE", default="m5stats.txt",
    help="Sets the output file for statistics [Default: %default]")

# Debugging options
set_group("Debugging Options")
add_option("--debug-break", metavar="TIME[,TIME]", action='append', split=',',
    help="Cycle to create a breakpoint")
add_option("--remote-gdb-port", type='int', default=7000,
    help="Remote gdb base port")

# Tracing options
set_group("Trace Options")
add_option("--trace-help", action='store_true',
    help="Print help on trace flags")
add_option("--trace-flags", metavar="FLAG[,FLAG]", action='append', split=',',
    help="Sets the flags for tracing (-FLAG disables a flag)")
add_option("--trace-start", metavar="TIME", type='int',
    help="Start tracing at TIME (must be in ticks)")
add_option("--trace-file", metavar="FILE", default="cout",
    help="Sets the output file for tracing [Default: %default]")
add_option("--trace-ignore", metavar="EXPR", action='append', split=':',
    help="Ignore EXPR sim objects")

options = attrdict()
arguments = []

def usage(exitcode=None):
    parser.print_help()
    if exitcode is not None:
        sys.exit(exitcode)

def parse_args():
    _opts,args = parser.parse_args()
    opts = attrdict(_opts.__dict__)

    # setting verbose and quiet at the same time doesn't make sense
    if opts.verbose > 0 and opts.quiet > 0:
        usage(2)

    # store the verbosity in a single variable.  0 is default,
    # negative numbers represent quiet and positive values indicate verbose
    opts.verbose -= opts.quiet

    del opts.quiet

    options.update(opts)
    arguments.extend(args)
    return opts,args

def main():
    import defines
    import event
    import info
    import internal

    parse_args()

    done = False

    if options.build_info:
        done = True
        print 'Build information:'
        print
        print 'compiled %s' % internal.core.cvar.compileDate;
        print 'started %s' % datetime.datetime.now().ctime()
        print 'executing on %s' % socket.gethostname()
        print 'build options:'
        keys = defines.m5_build_env.keys()
        keys.sort()
        for key in keys:
            val = defines.m5_build_env[key]
            print '    %s = %s' % (key, val)
        print

    if options.copyright:
        done = True
        print info.LICENSE
        print

    if options.authors:
        done = True
        print 'Author information:'
        print
        print info.AUTHORS
        print

    if options.readme:
        done = True
        print 'Readme:'
        print
        print info.README
        print

    if options.release_notes:
        done = True
        print 'Release Notes:'
        print
        print info.RELEASE_NOTES
        print

    if options.trace_help:
        done = True
        print "Base Flags:"
        print_list(traceflags.baseFlags, indent=4)
        print
        print "Compound Flags:"
        for flag in traceflags.compoundFlags:
            if flag == 'All':
                continue
            print "    %s:" % flag
            print_list(traceflags.compoundFlagMap[flag], indent=8)
            print

    if done:
        sys.exit(0)

    if options.verbose >= 0:
        print "M5 Simulator System"
        print brief_copyright
        print
        print "M5 compiled %s" % internal.core.cvar.compileDate;
        print "M5 started %s" % datetime.datetime.now().ctime()
        print "M5 executing on %s" % socket.gethostname()
        print "command line:",
        for argv in sys.argv:
            print argv,
        print

    # check to make sure we can find the listed script
    if not arguments or not os.path.isfile(arguments[0]):
        if arguments and not os.path.isfile(arguments[0]):
            print "Script %s not found" % arguments[0]

        usage(2)

    # tell C++ about output directory
    internal.core.setOutputDir(options.outdir)

    # update the system path with elements from the -p option
    sys.path[0:0] = options.path

    import objects

    # set stats options
    internal.stats.initText(options.stats_file)

    # set debugging options
    internal.debug.setRemoteGDBPort(options.remote_gdb_port)
    for when in options.debug_break:
        internal.debug.schedBreakCycle(int(when))

    on_flags = []
    off_flags = []
    for flag in options.trace_flags:
        off = False
        if flag.startswith('-'):
            flag = flag[1:]
            off = True
        if flag not in traceflags.allFlags:
            print >>sys.stderr, "invalid trace flag '%s'" % flag
            sys.exit(1)

        if off:
            off_flags.append(flag)
        else:
            on_flags.append(flag)

    for flag in on_flags:
        internal.trace.set(flag)

    for flag in off_flags:
        internal.trace.clear(flag)

    if options.trace_start:
        def enable_trace():
            internal.trace.cvar.enabled = True
        event.create(enable_trace, int(options.trace_start))
    else:
        internal.trace.cvar.enabled = True

    internal.trace.output(options.trace_file)

    for ignore in options.trace_ignore:
        internal.trace.ignore(ignore)

    sys.argv = arguments
    sys.path = [ os.path.dirname(sys.argv[0]) ] + sys.path

    scope = { '__file__' : sys.argv[0],
              '__name__' : '__m5_main__' }

    # we want readline if we're doing anything interactive
    if options.interactive or options.pdb:
        exec "import readline" in scope

    # if pdb was requested, execfile the thing under pdb, otherwise,
    # just do the execfile normally
    if options.pdb:
        from pdb import Pdb
        debugger = Pdb()
        debugger.run('execfile("%s")' % sys.argv[0], scope)
    else:
        execfile(sys.argv[0], scope)

    # once the script is done
    if options.interactive:
        interact = code.InteractiveConsole(scope)
        interact.interact("M5 Interactive Console")

if __name__ == '__main__':
    from pprint import pprint

    parse_args()

    print 'opts:'
    pprint(options, indent=4)
    print

    print 'args:'
    pprint(arguments, indent=4)