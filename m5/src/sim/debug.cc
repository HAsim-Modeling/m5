/*
 * Copyright (c) 2003, 2004, 2005
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
 *          Steven K. Reinhardt
 */

#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

#include <string>
#include <vector>

#include "sim/debug.hh"
#include "sim/eventq.hh"
#include "sim/sim_events.hh"

using namespace std;

int remote_gdb_base_port = 7000;

void
debug_break()
{
#ifndef NDEBUG
    kill(getpid(), SIGTRAP);
#else
    cprintf("debug_break suppressed, compiled with NDEBUG\n");
#endif
}

//
// Debug event: place a breakpoint on the process function and
// schedule the event to break at a particular cycle
//
class DebugBreakEvent : public Event
{
  public:

    DebugBreakEvent(EventQueue *q, Tick _when);

    void process();	// process event
    virtual const char *description() const;
};

//
// constructor: schedule at specified time
//
DebugBreakEvent::DebugBreakEvent(EventQueue *q, Tick _when)
    : Event(q, Debug_Break_Pri)
{
    setFlags(AutoDelete);
    schedule(_when);
}

//
// handle debug event: set debugger breakpoint on this function
//
void
DebugBreakEvent::process()
{
    debug_break();
}


const char *
DebugBreakEvent::description() const
{
    return "debug break";
}

//
// handy function to schedule DebugBreakEvent on main event queue
// (callable from debugger)
//
void
schedBreakCycle(Tick when)
{
    new DebugBreakEvent(&mainEventQueue, when);
}

void
eventqDump()
{
    mainEventQueue.dump();
}


//
// Set remote GDB base port.  0 means disable remote GDB.
// Callable from python.
//
void
setRemoteGDBPort(int port)
{
    remote_gdb_base_port = port;
}

int
getRemoteGDBPort()
{
    return remote_gdb_base_port;
}
