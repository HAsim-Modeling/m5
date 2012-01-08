/*
 * Copyright (c) 2003-2006 The Regents of The University of Michigan
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors: Nathan Binkert
 */

#ifndef __SIM_PSEUDO_INST_HH__
#define __SIM_PSEUDO_INST_HH__

class ThreadContext;

//We need the "Tick" and "Addr" data types from here
#include "base/types.hh"

namespace PseudoInst {

/**
 * @todo these externs are only here for a hack in fullCPU::takeOver...
 */
extern bool doStatisticsInsts;
extern bool doCheckpointInsts;
extern bool doQuiesce;

#if FULL_SYSTEM

void arm(ThreadContext *tc);
void quiesce(ThreadContext *tc);
void quiesceSkip(ThreadContext *tc);
void quiesceNs(ThreadContext *tc, uint64_t ns);
void quiesceCycles(ThreadContext *tc, uint64_t cycles);
uint64_t quiesceTime(ThreadContext *tc);
uint64_t readfile(ThreadContext *tc, Addr vaddr, uint64_t len,
    uint64_t offset);
void loadsymbol(ThreadContext *xc);
void addsymbol(ThreadContext *tc, Addr addr, Addr symbolAddr);
uint64_t initParam(ThreadContext *xc);

#else

static inline void
panicFsOnlyPseudoInst(const char *name)
{
    panic("Pseudo inst \"%s\" is only available in Full System mode.");
}

static inline void
arm(ThreadContext *tc)
{
    panicFsOnlyPseudoInst("arm");
}
static inline void
quiesce(ThreadContext *tc)
{
    panicFsOnlyPseudoInst("quiesce");
}
static inline void
quiesceSkip(ThreadContext *tc)
{
    panicFsOnlyPseudoInst("quiesceSkip");
}
static inline void
quiesceNs(ThreadContext *tc, uint64_t ns)
{
    panicFsOnlyPseudoInst("quiesceNs");
}
static inline void
quiesceCycles(ThreadContext *tc, uint64_t cycles)
{
    panicFsOnlyPseudoInst("quiesceCycles");
}
static inline uint64_t
quiesceTime(ThreadContext *tc)
{
    panicFsOnlyPseudoInst("quiesceTime");
    return 0;
}
static inline uint64_t
readfile(ThreadContext *tc, Addr vaddr, uint64_t len, uint64_t offset)
{
    panicFsOnlyPseudoInst("readFile");
    return 0;
}
static inline void
loadsymbol(ThreadContext *xc)
{
    panicFsOnlyPseudoInst("loadSymbol");
}
static inline void
addsymbol(ThreadContext *tc, Addr addr, Addr symbolAddr)
{
    panicFsOnlyPseudoInst("addSymbol");
}
static inline uint64_t
initParam(ThreadContext *tc)
{
    panicFsOnlyPseudoInst("initParam");
    return 0;
}

#endif

uint64_t rpns(ThreadContext *tc);
void wakeCPU(ThreadContext *tc, uint64_t cpuid);
void m5exit(ThreadContext *tc, Tick delay);
void resetstats(ThreadContext *tc, Tick delay, Tick period);
void dumpstats(ThreadContext *tc, Tick delay, Tick period);
void dumpresetstats(ThreadContext *tc, Tick delay, Tick period);
void m5checkpoint(ThreadContext *tc, Tick delay, Tick period);
void debugbreak(ThreadContext *tc);
void switchcpu(ThreadContext *tc);
void workbegin(ThreadContext *tc, uint64_t workid, uint64_t threadid);
void workend(ThreadContext *tc, uint64_t workid, uint64_t threadid);

} // namespace PseudoInst

#endif // __SIM_PSEUDO_INST_HH__