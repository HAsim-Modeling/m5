/*
 * Copyright (c) 2004, 2005
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
 * Authors: Lisa R. Hsu
 *          Nathan L. Binkert
 */

#ifndef __ARCH_ALPHA_KERNEL_STATS_HH__
#define __ARCH_ALPHA_KERNEL_STATS_HH__

#include <map>
#include <stack>
#include <string>
#include <vector>

#include "cpu/static_inst.hh"
#include "kern/kernel_stats.hh"

class BaseCPU;
class ThreadContext;
class FnEvent;
// What does kernel stats expect is included?
class System;

namespace AlphaISA {
namespace Kernel {

enum cpu_mode { kernel, user, idle, cpu_mode_num };
extern const char *modestr[];

class Statistics : public ::Kernel::Statistics
{
  protected:
    Addr idleProcess;
    cpu_mode themode;
    Tick lastModeTick;

    void changeMode(cpu_mode newmode, ThreadContext *tc);

  private:
    Stats::Vector<> _callpal;
//    Stats::Vector<> _faults;

    Stats::Vector<> _mode;
    Stats::Vector<> _modeGood;
    Stats::Formula _modeFraction;
    Stats::Vector<> _modeTicks;

    Stats::Scalar<> _swap_context;

  public:
    Statistics(System *system);

    void regStats(const std::string &name);

  public:
    void mode(cpu_mode newmode, ThreadContext *tc);
    void context(Addr oldpcbb, Addr newpcbb, ThreadContext *tc);
    void callpal(int code, ThreadContext *tc);
    void hwrei() { _hwrei++; }

    void setIdleProcess(Addr idle, ThreadContext *tc);

  public:
    void serialize(std::ostream &os);
    void unserialize(Checkpoint *cp, const std::string &section);
};

} /* end namespace AlphaISA::Kernel */
} /* end namespace AlphaISA */

#endif // __ARCH_ALPHA_KERNEL_STATS_HH__
