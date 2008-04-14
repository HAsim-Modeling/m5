/*
 * Copyright (c) 2003, 2004, 2005, 2006
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

class ThreadContext;

//We need the "Tick" and "Addr" data types from here
#include "sim/host.hh"

namespace PseudoInst
{
    /**
     * @todo these externs are only here for a hack in fullCPU::takeOver...
     */
    extern bool doStatisticsInsts;
    extern bool doCheckpointInsts;
    extern bool doQuiesce;

    void arm(ThreadContext *tc);
    void quiesce(ThreadContext *tc);
    void quiesceNs(ThreadContext *tc, uint64_t ns);
    void quiesceCycles(ThreadContext *tc, uint64_t cycles);
    uint64_t quiesceTime(ThreadContext *tc);
    void m5exit(ThreadContext *tc, Tick delay);
    void m5exit_old(ThreadContext *tc);
    void loadsymbol(ThreadContext *xc);
    void resetstats(ThreadContext *tc, Tick delay, Tick period);
    void dumpstats(ThreadContext *tc, Tick delay, Tick period);
    void dumpresetstats(ThreadContext *tc, Tick delay, Tick period);
    void m5checkpoint(ThreadContext *tc, Tick delay, Tick period);
    uint64_t readfile(ThreadContext *tc, Addr vaddr, uint64_t len, uint64_t offset);
    void debugbreak(ThreadContext *tc);
    void switchcpu(ThreadContext *tc);
    void addsymbol(ThreadContext *tc, Addr addr, Addr symbolAddr);
    void anBegin(ThreadContext *tc, uint64_t cur);
    void anWait(ThreadContext *tc, uint64_t cur, uint64_t wait);
}
