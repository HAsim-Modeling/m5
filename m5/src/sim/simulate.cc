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
 *          Steven K. Reinhardt
 */

#include "base/misc.hh"
#include "base/pollevent.hh"
#include "sim/stat_control.hh"
#include "sim/async.hh"
#include "sim/eventq.hh"
#include "sim/host.hh"
#include "sim/sim_events.hh"
#include "sim/sim_exit.hh"
#include "sim/simulate.hh"

/** Simulate for num_cycles additional cycles.  If num_cycles is -1
 * (the default), do not limit simulation; some other event must
 * terminate the loop.  Exported to Python via SWIG.
 * @return The SimLoopExitEvent that caused the loop to exit.
 */
SimLoopExitEvent *
simulate(Tick num_cycles)
{
    warn("Entering event queue @ %d.  Starting simulation...\n", curTick);

    if (num_cycles < 0)
        fatal("simulate: num_cycles must be >= 0 (was %d)\n", num_cycles);
    else if (curTick + num_cycles < 0)  //Overflow
        num_cycles = MaxTick;
    else
        num_cycles = curTick + num_cycles;

    Event *limit_event;
    limit_event = schedExitSimLoop("simulate() limit reached", num_cycles);

    while (1) {
        // there should always be at least one event (the SimLoopExitEvent
        // we just scheduled) in the queue
        assert(!mainEventQueue.empty());
        assert(curTick <= mainEventQueue.nextTick() &&
               "event scheduled in the past");

        // forward current cycle to the time of the first event on the
        // queue
        curTick = mainEventQueue.nextTick();
        Event *exit_event = mainEventQueue.serviceOne();
        if (exit_event != NULL) {
            // hit some kind of exit event; return to Python
            // event must be subclass of SimLoopExitEvent...
            SimLoopExitEvent *se_event;
            se_event = dynamic_cast<SimLoopExitEvent *>(exit_event);

            if (se_event == NULL)
                panic("Bogus exit event class!");

            // if we didn't hit limit_event, delete it
            if (se_event != limit_event) {
                assert(limit_event->scheduled());
                limit_event->deschedule();
                delete limit_event;
            }

            return se_event;
        }

        if (async_event) {
            async_event = false;
            if (async_statdump || async_statreset) {
                Stats::StatEvent(async_statdump, async_statreset);
                async_statdump = false;
                async_statreset = false;
            }

            if (async_exit) {
                async_exit = false;
                exitSimLoop("user interrupt received");
            }

            if (async_io || async_alarm) {
                async_io = false;
                async_alarm = false;
                pollQueue.service();
            }

            if (async_exception) {
                async_exception = false;
                return NULL;
            }
        }
    }

    // not reached... only exit is return on SimLoopExitEvent
}

