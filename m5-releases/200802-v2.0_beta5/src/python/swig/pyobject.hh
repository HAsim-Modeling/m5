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

#include "cpu/base.hh"
#include "sim/host.hh"
#include "sim/serialize.hh"
#include "sim/sim_object.hh"
#include "sim/system.hh"

extern "C" SimObject *convertSwigSimObjectPtr(PyObject *);
SimObject *resolveSimObject(const std::string &name);

/**
 * Connect the described MemObject ports.  Called from Python via SWIG.
 */
int connectPorts(SimObject *o1, const std::string &name1, int i1,
    SimObject *o2, const std::string &name2, int i2);

inline void
initAll()
{
    SimObject::initAll();
}

inline void
regAllStats()
{
    SimObject::regAllStats();
}

inline void
serializeAll(const std::string &cpt_dir)
{
    Serializable::serializeAll(cpt_dir);
}

inline void
unserializeAll(const std::string &cpt_dir)
{
    Serializable::unserializeAll(cpt_dir);
}

