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

#ifndef __SIM_CORE_HH__
#define __SIM_CORE_HH__

#include <string>

#include "sim/host.hh"

/// The universal simulation clock.
extern Tick curTick;
const Tick retryTime = 1000;

namespace Clock {
/// The simulated frequency of curTick.
extern Tick Frequency;

namespace Float {
extern double s;
extern double ms;
extern double us;
extern double ns;
extern double ps;

extern double Hz;
extern double kHz;
extern double MHz;
extern double GHZ;
/* namespace Float */ }

namespace Int {
extern Tick s;
extern Tick ms;
extern Tick us;
extern Tick ns;
extern Tick ps;
/* namespace Int */ }
/* namespace Clock */ }

void setClockFrequency(Tick ticksPerSecond);

/// Output stream for simulator messages (e.g., cprintf()).  Also used
/// as default stream for tracing and DPRINTF() messages (unless
/// overridden with trace:file option).
extern std::ostream *outputStream;
void setOutputFile(const std::string &file);
void setOutputDir(const std::string &dir);

struct Callback;
void registerExitCallback(Callback *callback);
void doExitCleanup();

#endif /* __SIM_CORE_HH__ */
