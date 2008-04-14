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

#ifndef __PYTHON_SWIG_PYEVENT_HH__
#define __PYTHON_SWIG_PYEVENT_HH__

#include "sim/eventq.hh"
#include "sim/sim_events.hh"

class PythonEvent : public Event
{
  private:
    PyObject *object;

  public:
    PythonEvent(PyObject *obj, Tick when, Priority priority = Default_Pri);
    ~PythonEvent();

    virtual void process();
};

inline void
create(PyObject *object, Tick when)
{
    new PythonEvent(object, when);
}

inline Event *
createCountedDrain()
{
    return new CountedDrainEvent();
}

inline void
cleanupCountedDrain(Event *counted_drain)
{
    CountedDrainEvent *event =
        dynamic_cast<CountedDrainEvent *>(counted_drain);
    if (event == NULL) {
        fatal("Called cleanupCountedDrain() on an event that was not "
              "a CountedDrainEvent.");
    }
    assert(event->getCount() == 0);
    delete event;
}

#endif // __PYTHON_SWIG_PYEVENT_HH__
