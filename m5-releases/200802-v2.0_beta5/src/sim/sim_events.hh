/*
 * Copyright (c) 2002, 2003, 2004, 2005
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

#ifndef __SIM_SIM_EVENTS_HH__
#define __SIM_SIM_EVENTS_HH__

#include "sim/eventq.hh"

//
// Event to terminate simulation at a particular cycle/instruction
//
class SimLoopExitEvent : public Event
{
  private:
    // string explaining why we're terminating
    std::string cause;
    int code;
    Tick repeat;

  public:
    // Default constructor.  Only really used for derived classes.
    SimLoopExitEvent()
        : Event(&mainEventQueue, Sim_Exit_Pri)
    { }

    SimLoopExitEvent(EventQueue *q,
                     Tick _when, Tick _repeat, const std::string &_cause,
                     int c = 0)
        : Event(q, Sim_Exit_Pri), cause(_cause),
          code(c), repeat(_repeat)
    { setFlags(IsExitEvent); schedule(_when); }

//     SimLoopExitEvent(EventQueue *q,
// 		     Tick _when, const std::string &_cause,
// 		     Tick _repeat = 0, int c = 0)
// 	: Event(q, Sim_Exit_Pri), cause(_cause), code(c), repeat(_repeat)
//     { setFlags(IsExitEvent); schedule(_when); }

    std::string getCause() { return cause; }
    int getCode() { return code; }

    void process();	// process event

    virtual const char *description() const;
};

class CountedDrainEvent : public SimLoopExitEvent
{
  private:
    // Count of how many objects have not yet drained
    int count;
  public:
    CountedDrainEvent()
        : count(0)
    { }
    void process();

    void setCount(int _count) { count = _count; }

    int getCount() { return count; }
};

//
// Event class to terminate simulation after 'n' related events have
// occurred using a shared counter: used to terminate when *all*
// threads have reached a particular instruction count
//
class CountedExitEvent : public Event
{
  private:
    std::string cause;	// string explaining why we're terminating
    int &downCounter;	// decrement & terminate if zero

  public:
    CountedExitEvent(EventQueue *q, const std::string &_cause,
                     Tick _when, int &_downCounter);

    void process();	// process event

    virtual const char *description() const;
};

//
// Event to check swap usage
//
class CheckSwapEvent : public Event
{
  private:
    int interval;

  public:
    CheckSwapEvent(EventQueue *q, int ival)
        : Event(q), interval(ival)
    { schedule(curTick + interval); }

    void process();	// process event

    virtual const char *description() const;
};

#endif  // __SIM_SIM_EVENTS_HH__
