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

import os
import sys

import smartdict

# define a MaxTick parameter
MaxTick = 2**63 - 1

# define this here so we can use it right away if necessary
def panic(string):
    print >>sys.stderr, 'panic:', string
    sys.exit(1)

# force scalars to one-element lists for uniformity
def makeList(objOrList):
    if isinstance(objOrList, list):
        return objOrList
    return [objOrList]

# Prepend given directory to system module search path.  We may not
# need this anymore if we can structure our config library more like a
# Python package.
def AddToPath(path):
    # if it's a relative path and we know what directory the current
    # python script is in, make the path relative to that directory.
    if not os.path.isabs(path) and sys.path[0]:
        path = os.path.join(sys.path[0], path)
    path = os.path.realpath(path)
    # sys.path[0] should always refer to the current script's directory,
    # so place the new dir right after that.
    sys.path.insert(1, path)

# make a SmartDict out of the build options for our local use
build_env = smartdict.SmartDict()

# make a SmartDict out of the OS environment too
env = smartdict.SmartDict()
env.update(os.environ)

# Since we have so many mutual imports in this package, we should:
# 1. Put all intra-package imports at the *bottom* of the file, unless
#    they're absolutely needed before that (for top-level statements
#    or class attributes).  Imports of "trivial" packages that don't
#    import other packages (e.g., 'smartdict') can be at the top.
# 2. Never use 'from foo import *' on an intra-package import since
#    you can get the wrong result if foo is only partially imported
#    at the point you do that (i.e., because foo is in the middle of
#    importing *you*).
try:
    import internal
    running_m5 = True
except ImportError:
    running_m5 = False

if running_m5:
    import defines
    build_env.update(defines.m5_build_env)
else:
    import __scons
    build_env.update(__scons.m5_build_env)

if running_m5:
    from event import *
    from simulate import *
    from main import options
    import stats

import SimObject
import params
import objects
