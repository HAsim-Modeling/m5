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

#include <string>

#include "base/trace.hh"
#include "cpu/thread_context.hh"
#include "kern/kernel_stats.hh"
#include "kern/tru64/tru64_syscalls.hh"
#include "sim/system.hh"

using namespace std;
using namespace Stats;

namespace Kernel {

Statistics::Statistics(System *system)
    : iplLast(0), iplLastTick(0)
{
}

void
Statistics::regStats(const string &_name)
{
    myname = _name;

    _arm
        .name(name() + ".inst.arm")
        .desc("number of arm instructions executed")
        ;

    _quiesce
        .name(name() + ".inst.quiesce")
        .desc("number of quiesce instructions executed")
        ;

    _iplCount
        .init(32)
        .name(name() + ".ipl_count")
        .desc("number of times we switched to this ipl")
        .flags(total | pdf | nozero | nonan)
        ;

    _iplGood
        .init(32)
        .name(name() + ".ipl_good")
        .desc("number of times we switched to this ipl from a different ipl")
        .flags(total | pdf | nozero | nonan)
        ;

    _iplTicks
        .init(32)
        .name(name() + ".ipl_ticks")
        .desc("number of cycles we spent at this ipl")
        .flags(total | pdf | nozero | nonan)
        ;

    _iplUsed
        .name(name() + ".ipl_used")
        .desc("fraction of swpipl calls that actually changed the ipl")
        .flags(total | nozero | nonan)
        ;

    _iplUsed = _iplGood / _iplCount;

    _syscall
        .init(SystemCalls<Tru64>::Number)
        .name(name() + ".syscall")
        .desc("number of syscalls executed")
        .flags(total | pdf | nozero | nonan)
        ;

    //@todo This needs to get the names of syscalls from an appropriate place.
#if 0
    for (int i = 0; i < SystemCalls<Tru64>::Number; ++i) {
        const char *str = SystemCalls<Tru64>::name(i);
        if (str) {
            _syscall.subname(i, str);
        }
    }
#endif
}

void
Statistics::swpipl(int ipl)
{
    assert(ipl >= 0 && ipl <= 0x1f && "invalid IPL\n");

    _iplCount[ipl]++;

    if (ipl == iplLast)
        return;

    _iplGood[ipl]++;
    _iplTicks[iplLast] += curTick - iplLastTick;
    iplLastTick = curTick;
    iplLast = ipl;
}

void
Statistics::serialize(ostream &os)
{
    SERIALIZE_SCALAR(iplLast);
    SERIALIZE_SCALAR(iplLastTick);
}

void
Statistics::unserialize(Checkpoint *cp, const string &section)
{
    UNSERIALIZE_SCALAR(iplLast);
    UNSERIALIZE_SCALAR(iplLastTick);
}

/* end namespace Kernel */ }
