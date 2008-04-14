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

%module sim_object

%{
#include "python/swig/pyobject.hh"
%}

// import these files for SWIG to wrap
%include "stdint.i"
%include "std_string.i"
%include "sim/host.hh"

class BaseCPU;

class SimObject {
  public:
    enum State {
      Running,
      Draining,
      Drained
    };

    unsigned int drain(Event *drain_event);
    void resume();
    void switchOut();
    void takeOverFrom(BaseCPU *cpu);
    SimObject(const SimObjectParams *p);
};

int connectPorts(SimObject *o1, const std::string &name1, int i1,
                 SimObject *o2, const std::string &name2, int i2);

%wrapper %{
// Convert a pointer to the Python object that SWIG wraps around a
// C++ SimObject pointer back to the actual C++ pointer.
SimObject *
convertSwigSimObjectPtr(PyObject *pyObj)
{
    SimObject *so;
    if (SWIG_ConvertPtr(pyObj, (void **) &so, SWIGTYPE_p_SimObject, 0) == -1)
        return NULL;
    return so;
}
%}
