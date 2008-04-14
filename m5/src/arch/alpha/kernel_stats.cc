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

#include <map>
#include <stack>
#include <string>

#include "arch/alpha/linux/threadinfo.hh"
#include "arch/alpha/kernel_stats.hh"
#include "arch/alpha/osfpal.hh"
#include "base/trace.hh"
#include "cpu/thread_context.hh"
#include "kern/tru64/tru64_syscalls.hh"
#include "sim/system.hh"

using namespace std;
using namespace Stats;

namespace AlphaISA {
namespace Kernel {

const char *modestr[] = { "kernel", "user", "idle" };

Statistics::Statistics(System *system)
    : ::Kernel::Statistics(system),
      idleProcess((Addr)-1), themode(kernel), lastModeTick(0)
{
}

void
Statistics::regStats(const string &_name)
{
    ::Kernel::Statistics::regStats(_name);

    _callpal
        .init(256)
        .name(name() + ".callpal")
        .desc("number of callpals executed")
        .flags(total | pdf | nozero | nonan)
        ;

    for (int i = 0; i < PAL::NumCodes; ++i) {
        const char *str = PAL::name(i);
        if (str)
            _callpal.subname(i, str);
    }

    _hwrei
        .name(name() + ".inst.hwrei")
        .desc("number of hwrei instructions executed")
        ;

    _mode
        .init(cpu_mode_num)
        .name(name() + ".mode_switch")
        .desc("number of protection mode switches")
        ;

    for (int i = 0; i < cpu_mode_num; ++i)
        _mode.subname(i, modestr[i]);

    _modeGood
        .init(cpu_mode_num)
        .name(name() + ".mode_good")
        ;

    for (int i = 0; i < cpu_mode_num; ++i)
        _modeGood.subname(i, modestr[i]);

    _modeFraction
        .name(name() + ".mode_switch_good")
        .desc("fraction of useful protection mode switches")
        .flags(total)
        ;

    for (int i = 0; i < cpu_mode_num; ++i)
        _modeFraction.subname(i, modestr[i]);

    _modeFraction = _modeGood / _mode;

    _modeTicks
        .init(cpu_mode_num)
        .name(name() + ".mode_ticks")
        .desc("number of ticks spent at the given mode")
        .flags(pdf)
        ;
    for (int i = 0; i < cpu_mode_num; ++i)
        _modeTicks.subname(i, modestr[i]);

    _swap_context
        .name(name() + ".swap_context")
        .desc("number of times the context was actually changed")
        ;
}

void
Statistics::setIdleProcess(Addr idlepcbb, ThreadContext *tc)
{
    assert(themode == kernel);
    idleProcess = idlepcbb;
    themode = idle;
    changeMode(themode, tc);
}

void
Statistics::changeMode(cpu_mode newmode, ThreadContext *tc)
{
    _mode[newmode]++;

    if (newmode == themode)
        return;

    DPRINTF(Context, "old mode=%s new mode=%s pid=%d\n",
            modestr[themode], modestr[newmode],
            Linux::ThreadInfo(tc).curTaskPID());

    _modeGood[newmode]++;
    _modeTicks[themode] += curTick - lastModeTick;

    lastModeTick = curTick;
    themode = newmode;
}

void
Statistics::mode(cpu_mode newmode, ThreadContext *tc)
{
    Addr pcbb = tc->readMiscRegNoEffect(AlphaISA::IPR_PALtemp23);

    if (newmode == kernel && pcbb == idleProcess)
        newmode = idle;

    changeMode(newmode, tc);
}

void
Statistics::context(Addr oldpcbb, Addr newpcbb, ThreadContext *tc)
{
    assert(themode != user);

    _swap_context++;
    changeMode(newpcbb == idleProcess ? idle : kernel, tc);

    DPRINTF(Context, "Context Switch old pid=%d new pid=%d\n",
            Linux::ThreadInfo(tc, oldpcbb).curTaskPID(),
            Linux::ThreadInfo(tc, newpcbb).curTaskPID());
}

void
Statistics::callpal(int code, ThreadContext *tc)
{
    if (!PAL::name(code))
        return;

    _callpal[code]++;

    switch (code) {
      case PAL::callsys: {
          int number = tc->readIntReg(0);
          if (SystemCalls<Tru64>::validSyscallNumber(number)) {
              int cvtnum = SystemCalls<Tru64>::convert(number);
              _syscall[cvtnum]++;
          }
      } break;
    }
}

void
Statistics::serialize(ostream &os)
{
    ::Kernel::Statistics::serialize(os);
    int exemode = themode;
    SERIALIZE_SCALAR(exemode);
    SERIALIZE_SCALAR(idleProcess);
    SERIALIZE_SCALAR(lastModeTick);
}

void
Statistics::unserialize(Checkpoint *cp, const string &section)
{
    ::Kernel::Statistics::unserialize(cp, section);
    int exemode;
    UNSERIALIZE_SCALAR(exemode);
    UNSERIALIZE_SCALAR(idleProcess);
    UNSERIALIZE_SCALAR(lastModeTick);
    themode = (cpu_mode)exemode;
}

} /* end namespace AlphaISA::Kernel */
} /* end namespace AlphaISA */
