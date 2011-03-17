/*
 * Copyright (c) 2006
 * The Regents of The University of Michigan
 * All Rights Reserved
 *
 * This code is part of the M5 simulator.
 *
 * Permission is granted to use, copy, create derivative works and
 * redistribute this software and such derivative works for any
 * purpose, so long as the copyright notice above, this grant of
 * permission, and the disclaimer below appear in all copies made; and
 * so long as the name of The University of Michigan is not used in
 * any advertising or publicity pertaining to the use or distribution
 * of this software without specific, written prior authorization.
 *
 * THIS SOFTWARE IS PROVIDED AS IS, WITHOUT REPRESENTATION FROM THE
 * UNIVERSITY OF MICHIGAN AS TO ITS FITNESS FOR ANY PURPOSE, AND
 * WITHOUT WARRANTY BY THE UNIVERSITY OF MICHIGAN OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE. THE REGENTS OF THE UNIVERSITY OF MICHIGAN SHALL NOT BE
 * LIABLE FOR ANY DAMAGES, INCLUDING DIRECT, SPECIAL, INDIRECT,
 * INCIDENTAL, OR CONSEQUENTIAL DAMAGES, WITH RESPECT TO ANY CLAIM
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OF THE SOFTWARE, EVEN
 * IF IT HAS BEEN OR IS HEREAFTER ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGES.
 *
 * Authors: Nathan L. Binkert
 */

#include <Python.h>

#include <string>

#include "base/inifile.hh"
#include "base/output.hh"
#include "config/full_system.hh"
#include "mem/mem_object.hh"
#include "mem/port.hh"
#include "sim/sim_object.hh"

#if FULL_SYSTEM
#include "dev/etherdevice.hh"
#include "dev/etherobject.hh"
#include "dev/etherint.hh"
#endif

using namespace std;

/**
 * Look up a MemObject port.  Helper function for connectPorts().
 */
Port *
lookupPort(SimObject *so, const std::string &name, int i)
{
    MemObject *mo = dynamic_cast<MemObject *>(so);
    if (mo == NULL) {
        warn("error casting SimObject %s to MemObject", so->name());
        return NULL;
    }

    Port *p = mo->getPort(name, i);
    if (p == NULL)
        warn("error looking up port %s on object %s", name, so->name());
    return p;
}

#if FULL_SYSTEM

EtherInt *
lookupEthPort(SimObject *so, const std::string &name, int i)
{
    EtherObject *eo = dynamic_cast<EtherObject *>(so);
    EtherDevice *ed = dynamic_cast<EtherDevice *>(so);
    if (eo == NULL && ed == NULL) {
        warn("error casting SimObject %s", so->name());
        return NULL;
    }

    EtherInt *p = NULL;
    if (eo)
        p = eo->getEthPort(name, i);
    else
        p = ed->getEthPort(name, i);
    return p;
}
#endif

/**
 * Connect the described MemObject ports.  Called from Python via SWIG.
 * The indices i1 & i2 will be -1 for regular ports, >= 0 for vector ports.
 */
int
connectPorts(SimObject *o1, const std::string &name1, int i1,
             SimObject *o2, const std::string &name2, int i2)
{
    MemObject *mo1, *mo2;
    mo1 = dynamic_cast<MemObject*>(o1);
    mo2 = dynamic_cast<MemObject*>(o2);

#if FULL_SYSTEM
    EtherObject *eo1, *eo2;
    EtherDevice *ed1, *ed2;
    eo1 = dynamic_cast<EtherObject*>(o1);
    ed1 = dynamic_cast<EtherDevice*>(o1);

    eo2 = dynamic_cast<EtherObject*>(o2);
    ed2 = dynamic_cast<EtherDevice*>(o2);

    if ((eo1 || ed1) && (eo2 || ed2)) {
        EtherInt *p1 = lookupEthPort(o1, name1, i1);
        EtherInt *p2 = lookupEthPort(o2, name2, i2);

        if (p1 != NULL &&  p2 != NULL) {

            p1->setPeer(p2);
            p2->setPeer(p1);

            return 1;
        }
    }
#endif
    Port *p1 = lookupPort(o1, name1, i1);
    Port *p2 = lookupPort(o2, name2, i2);

    if (p1 == NULL || p2 == NULL) {
        warn("connectPorts: port lookup error");
        return 0;
    }

    p1->setPeer(p2);
    p2->setPeer(p1);

    return 1;
}

inline IniFile &
inifile()
{
    static IniFile inifile;
    return inifile;
}

/**
 * Convert a pointer to the Python object that SWIG wraps around a C++
 * SimObject pointer back to the actual C++ pointer.  See main.i.
 */
extern "C" SimObject *convertSwigSimObjectPtr(PyObject *);

// Python.h is notoriously not const-correct (for 2.4, anyway)... make
// a little define here to reduce the noise and make it easier to
// #ifdef away if Python.h gets fixed.  Note there are a couple of
// these in sim/main.cc as well that are handled without this define.
#define PCC(s)  const_cast<char *>(s)


SimObject *
resolveSimObject(const string &name)
{
    PyObject *module = PyImport_ImportModule(PCC("m5.SimObject"));
    if (module == NULL)
        panic("Could not import m5.SimObject");

    PyObject *resolver =
        PyObject_GetAttrString(module, PCC("resolveSimObject"));
    if (resolver == NULL) {
        PyErr_Print();
        panic("resolveSimObject: failed to find resolveSimObject");
    }

    PyObject *ptr = PyObject_CallFunction(resolver, PCC("(s)"), name.c_str());
    if (ptr == NULL) {
        PyErr_Print();
        panic("resolveSimObject: failure on call to Python for %s", name);
    }

    SimObject *obj = convertSwigSimObjectPtr(ptr);
    if (obj == NULL)
        panic("resolveSimObject: failure on pointer conversion for %s", name);

    Py_DECREF(module);
    Py_DECREF(resolver);
    Py_DECREF(ptr);

    return obj;
}
