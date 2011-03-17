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

#####################################################################
#
# Proxy object support.
#
#####################################################################

import copy

class BaseProxy(object):
    def __init__(self, search_self, search_up):
        self._search_self = search_self
        self._search_up = search_up
        self._multiplier = None

    def __str__(self):
        if self._search_self and not self._search_up:
            s = 'Self'
        elif not self._search_self and self._search_up:
            s = 'Parent'
        else:
            s = 'ConfusedProxy'
        return s + '.' + self.path()

    def __setattr__(self, attr, value):
        if not attr.startswith('_'):
            raise AttributeError, \
                  "cannot set attribute '%s' on proxy object" % attr
        super(BaseProxy, self).__setattr__(attr, value)

    # support multiplying proxies by constants
    def __mul__(self, other):
        if not isinstance(other, (int, long, float)):
            raise TypeError, "Proxy multiplier must be integer"
        if self._multiplier == None:
            self._multiplier = other
        else:
            # support chained multipliers
            self._multiplier *= other
        return self

    __rmul__ = __mul__

    def _mulcheck(self, result):
        if self._multiplier == None:
            return result
        return result * self._multiplier

    def unproxy(self, base):
        obj = base
        done = False

        if self._search_self:
            result, done = self.find(obj)

        if self._search_up:
            while not done:
                obj = obj._parent
                if not obj:
                    break
                result, done = self.find(obj)

        if not done:
            raise AttributeError, \
                  "Can't resolve proxy '%s' of type '%s' from '%s'" % \
                  (self.path(), self._pdesc.ptype_str, base.path())

        if isinstance(result, BaseProxy):
            if result == self:
                raise RuntimeError, "Cycle in unproxy"
            result = result.unproxy(obj)

        return self._mulcheck(result)

    def getindex(obj, index):
        if index == None:
            return obj
        try:
            obj = obj[index]
        except TypeError:
            if index != 0:
                raise
            # if index is 0 and item is not subscriptable, just
            # use item itself (so cpu[0] works on uniprocessors)
        return obj
    getindex = staticmethod(getindex)

    # This method should be called once the proxy is assigned to a
    # particular parameter or port to set the expected type of the
    # resolved proxy
    def set_param_desc(self, pdesc):
        self._pdesc = pdesc

class AttrProxy(BaseProxy):
    def __init__(self, search_self, search_up, attr):
        super(AttrProxy, self).__init__(search_self, search_up)
        self._attr = attr
        self._modifiers = []

    def __getattr__(self, attr):
        # python uses __bases__ internally for inheritance
        if attr.startswith('_'):
            return super(AttrProxy, self).__getattr__(self, attr)
        if hasattr(self, '_pdesc'):
            raise AttributeError, "Attribute reference on bound proxy"
        # Return a copy of self rather than modifying self in place
        # since self could be an indirect reference via a variable or
        # parameter
        new_self = copy.deepcopy(self)
        new_self._modifiers.append(attr)
        return new_self

    # support indexing on proxies (e.g., Self.cpu[0])
    def __getitem__(self, key):
        if not isinstance(key, int):
            raise TypeError, "Proxy object requires integer index"
        if hasattr(self, '_pdesc'):
            raise AttributeError, "Index operation on bound proxy"
        new_self = copy.deepcopy(self)
        new_self._modifiers.append(key)
        return new_self

    def find(self, obj):
        try:
            val = getattr(obj, self._attr)
        except:
            return None, False
        while isproxy(val):
            val = val.unproxy(obj)
        for m in self._modifiers:
            if isinstance(m, str):
                val = getattr(val, m)
            elif isinstance(m, int):
                val = val[m]
            else:
                assert("Item must be string or integer")
            while isproxy(val):
                val = val.unproxy(obj)
        return val, True

    def path(self):
        p = self._attr
        for m in self._modifiers:
            if isinstance(m, str):
                p += '.%s' % m
            elif isinstance(m, int):
                p += '[%d]' % m
            else:
                assert("Item must be string or integer")
        return p

class AnyProxy(BaseProxy):
    def find(self, obj):
        return obj.find_any(self._pdesc.ptype)

    def path(self):
        return 'any'

def isproxy(obj):
    if isinstance(obj, (BaseProxy, params.EthernetAddr)):
        return True
    elif isinstance(obj, (list, tuple)):
        for v in obj:
            if isproxy(v):
                return True
    return False

class ProxyFactory(object):
    def __init__(self, search_self, search_up):
        self.search_self = search_self
        self.search_up = search_up

    def __getattr__(self, attr):
        if attr == 'any':
            return AnyProxy(self.search_self, self.search_up)
        else:
            return AttrProxy(self.search_self, self.search_up, attr)

# global objects for handling proxies
Parent = ProxyFactory(search_self = False, search_up = True)
Self = ProxyFactory(search_self = True, search_up = False)

# limit exports on 'from proxy import *'
__all__ = ['Parent', 'Self']

# see comment on imports at end of __init__.py.
import params # for EthernetAddr
