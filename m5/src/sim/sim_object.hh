/*
 * Copyright (c) 2001, 2002, 2003, 2004, 2005
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
 * Authors: Steven K. Reinhardt
 *          Nathan L. Binkert
 */

/* @file
 * User Console Definitions
 */

#ifndef __SIM_OBJECT_HH__
#define __SIM_OBJECT_HH__

#include <map>
#include <list>
#include <vector>
#include <iostream>

#include "params/SimObject.hh"
#include "sim/serialize.hh"
#include "sim/startup.hh"

class BaseCPU;
class Event;

/*
 * Abstract superclass for simulation objects.  Represents things that
 * correspond to physical components and can be specified via the
 * config file (CPUs, caches, etc.).
 */
class SimObject : public Serializable, protected StartupCallback
{
  public:
    enum State {
        Running,
        Draining,
        Drained
    };

  private:
    State state;

  protected:
    void changeState(State new_state) { state = new_state; }

  public:
    State getState() { return state; }

  private:
    typedef std::vector<SimObject *> SimObjectList;

    // list of all instantiated simulation objects
    static SimObjectList simObjectList;

  protected:
    const SimObjectParams *_params;

  public:
    typedef SimObjectParams Params;
    const Params *params() const { return _params; }
    SimObject(const Params *_params);
    virtual ~SimObject() {}

  protected:
    // static: support for old-style constructors (call manually)
    static Params *makeParams(const std::string &name);

  public:

    virtual const std::string name() const { return params()->name; }

    // initialization pass of all objects.
    // Gets invoked after construction, before unserialize.
    virtual void init();
    static void initAll();

    // register statistics for this object
    virtual void regStats();
    virtual void regFormulas();
    virtual void resetStats();

    // static: call reg_stats on all SimObjects
    static void regAllStats();

    // static: call resetStats on all SimObjects
    static void resetAllStats();

    // static: call nameOut() & serialize() on all SimObjects
    static void serializeAll(std::ostream &);
    static void unserializeAll(Checkpoint *cp);

    // Methods to drain objects in order to take checkpoints
    // Or switch from timing -> atomic memory model
    // Drain returns 0 if the simobject can drain immediately or
    // the number of times the drain_event's process function will be called
    // before the object will be done draining. Normally this should be 1
    virtual unsigned int drain(Event *drain_event);
    virtual void resume();
    virtual void setMemoryMode(State new_mode);
    virtual void switchOut();
    virtual void takeOverFrom(BaseCPU *cpu);

#ifdef DEBUG
  public:
    bool doDebugBreak;
    static void debugObjectBreak(const std::string &objs);
#endif

    /**
     * Find the SimObject with the given name and return a pointer to
     * it.  Primarily used for interactive debugging.  Argument is
     * char* rather than std::string to make it callable from gdb.
     */
    static SimObject *find(const char *name);

  public:
    void recordEvent(const std::string &stat);
};

#endif // __SIM_OBJECT_HH__
