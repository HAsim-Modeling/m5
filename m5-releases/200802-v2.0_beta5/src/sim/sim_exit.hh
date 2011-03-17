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

#ifndef __SIM_EXIT_HH__
#define __SIM_EXIT_HH__

#include <string>

#include "sim/host.hh"

// forward declaration
class Callback;
class EventQueue;
class SimLoopExitEvent;

/// Register a callback to be called when Python exits.  Defined in
/// sim/main.cc.
void registerExitCallback(Callback *);

/// Schedule an event to exit the simulation loop (returning to
/// Python) at the indicated tick.  The message and exit_code
/// parameters are saved in the SimLoopExitEvent to indicate why the
/// exit occurred.
SimLoopExitEvent *schedExitSimLoop(const std::string &message, Tick when,
                                   Tick repeat = 0, EventQueue *q = NULL,
                                   int exit_code = 0);

/// Schedule an event to exit the simulation loop (returning to
/// Python) at the end of the current cycle (curTick).  The message
/// and exit_code parameters are saved in the SimLoopExitEvent to
/// indicate why the exit occurred.
void exitSimLoop(const std::string &message, int exit_code = 0);

#endif // __SIM_EXIT_HH__
