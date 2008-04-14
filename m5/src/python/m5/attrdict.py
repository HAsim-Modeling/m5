# Copyright (c) 2006
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

__all__ = [ 'attrdict' ]

class attrdict(dict):
    def __getattr__(self, attr):
        if attr in self:
            return self.__getitem__(attr)
        return super(attrdict, self).__getattribute__(attr)

    def __setattr__(self, attr, value):
        if attr in dir(self):
            return super(attrdict, self).__setattr__(attr, value)
        return self.__setitem__(attr, value)

    def __delattr__(self, attr):
        if attr in self:
            return self.__delitem__(attr)
        return super(attrdict, self).__delattr__(attr, value)

if __name__ == '__main__':
    x = attrdict()
    x.y = 1
    x['z'] = 2
    print x['y'], x.y
    print x['z'], x.z
    print dir(x)
    print x

    print

    del x['y']
    del x.z
    print dir(x)
    print(x)
