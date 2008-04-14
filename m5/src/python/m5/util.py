# Copyright (c) 2004, 2005, 2006
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
# Authors: Steven K. Reinhardt
#          Nathan L. Binkert

#############################
#
# Utility classes & methods
#
#############################

class Singleton(type):
    def __call__(cls, *args, **kwargs):
        if hasattr(cls, '_instance'):
            return cls._instance

        cls._instance = super(Singleton, cls).__call__(*args, **kwargs)
        return cls._instance

# Apply method to object.
# applyMethod(obj, 'meth', <args>) is equivalent to obj.meth(<args>)
def applyMethod(obj, meth, *args, **kwargs):
    return getattr(obj, meth)(*args, **kwargs)

# If the first argument is an (non-sequence) object, apply the named
# method with the given arguments.  If the first argument is a
# sequence, apply the method to each element of the sequence (a la
# 'map').
def applyOrMap(objOrSeq, meth, *args, **kwargs):
    if not isinstance(objOrSeq, (list, tuple)):
        return applyMethod(objOrSeq, meth, *args, **kwargs)
    else:
        return [applyMethod(o, meth, *args, **kwargs) for o in objOrSeq]


