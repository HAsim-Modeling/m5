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
 * Authors: Kevin T. Lim
 *          Korey L. Sewell
 */

#include "config/use_checker.hh"

#include "arch/mips/faults.hh"
#include "base/cprintf.hh"
#include "base/statistics.hh"
#include "base/timebuf.hh"
#include "cpu/checker/thread_context.hh"
#include "sim/sim_events.hh"
#include "sim/stats.hh"

#include "cpu/o3/mips/cpu.hh"
#include "cpu/o3/mips/params.hh"
#include "cpu/o3/mips/thread_context.hh"
#include "cpu/o3/comm.hh"
#include "cpu/o3/thread_state.hh"

template <class Impl>
MipsO3CPU<Impl>::MipsO3CPU(Params *params)
    : FullO3CPU<Impl>(this, params)
{
    DPRINTF(O3CPU, "Creating MipsO3CPU object.\n");

    // Setup any thread state.
    this->thread.resize(this->numThreads);

    for (int i = 0; i < this->numThreads; ++i) {
        if (i < params->workload.size()) {
            DPRINTF(O3CPU, "Workload[%i] process is %#x",
                    i, this->thread[i]);
            this->thread[i] = new Thread(this, i, params->workload[i], i);

            this->thread[i]->setStatus(ThreadContext::Suspended);

            //usedTids[i] = true;
            //threadMap[i] = i;
        } else {
            //Allocate Empty thread so M5 can use later
            //when scheduling threads to CPU
            Process* dummy_proc = NULL;

            this->thread[i] = new Thread(this, i, dummy_proc, i);
            //usedTids[i] = false;
        }

        ThreadContext *tc;

        // Setup the TC that will serve as the interface to the threads/CPU.
        MipsTC<Impl> *mips_tc =
            new MipsTC<Impl>;

        tc = mips_tc;

        // If we're using a checker, then the TC should be the
        // CheckerThreadContext.
#if USE_CHECKER
        if (params->checker) {
            tc = new CheckerThreadContext<MipsTC<Impl> >(
                mips_tc, this->checker);
        }
#endif

        mips_tc->cpu = this;
        mips_tc->thread = this->thread[i];

        // Give the thread the TC.
        this->thread[i]->tc = tc;
        this->thread[i]->setCpuId(params->cpu_id);

        // Add the TC to the CPU's list of TC's.
        this->threadContexts.push_back(tc);
    }

    for (int i=0; i < this->numThreads; i++) {
        this->thread[i]->setFuncExeInst(0);
    }

    lockAddr = 0;
    lockFlag = false;
}

template <class Impl>
void
MipsO3CPU<Impl>::regStats()
{
    // Register stats for everything that has stats.
    this->fullCPURegStats();
    this->fetch.regStats();
    this->decode.regStats();
    this->rename.regStats();
    this->iew.regStats();
    this->commit.regStats();
}


template <class Impl>
MiscReg
MipsO3CPU<Impl>::readMiscRegNoEffect(int misc_reg, unsigned tid)
{
    return this->regFile.readMiscRegNoEffect(misc_reg, tid);
}

template <class Impl>
MiscReg
MipsO3CPU<Impl>::readMiscReg(int misc_reg, unsigned tid)
{
    return this->regFile.readMiscReg(misc_reg, tid);
}

template <class Impl>
void
MipsO3CPU<Impl>::setMiscRegNoEffect(int misc_reg, const MiscReg &val, unsigned tid)
{
    this->regFile.setMiscRegNoEffect(misc_reg, val, tid);
}

template <class Impl>
void
MipsO3CPU<Impl>::setMiscReg(int misc_reg, const MiscReg &val,
                                       unsigned tid)
{
    this->regFile.setMiscReg(misc_reg, val, tid);
}

template <class Impl>
void
MipsO3CPU<Impl>::squashFromTC(unsigned tid)
{
    this->thread[tid]->inSyscall = true;
    this->commit.generateTCEvent(tid);
}

template <class Impl>
void
MipsO3CPU<Impl>::trap(Fault fault, unsigned tid)
{
    // Pass the thread's TC into the invoke method.
    fault->invoke(this->threadContexts[tid]);
}

#if !FULL_SYSTEM

template <class Impl>
void
MipsO3CPU<Impl>::syscall(int64_t callnum, int tid)
{
    DPRINTF(O3CPU, "[tid:%i] Executing syscall().\n\n", tid);

    DPRINTF(Activity,"Activity: syscall() called.\n");

    // Temporarily increase this by one to account for the syscall
    // instruction.
    ++(this->thread[tid]->funcExeInst);

    // Execute the actual syscall.
    this->thread[tid]->syscall(callnum);

    // Decrease funcExeInst by one as the normal commit will handle
    // incrementing it.
    --(this->thread[tid]->funcExeInst);

    DPRINTF(O3CPU, "[tid:%i] Register 2 is %i ", tid, this->readIntReg(2));
}

template <class Impl>
TheISA::IntReg
MipsO3CPU<Impl>::getSyscallArg(int i, int tid)
{
    assert(i < TheISA::NumArgumentRegs);
    return this->readArchIntReg(MipsISA::ArgumentReg[i], tid);
}

template <class Impl>
void
MipsO3CPU<Impl>::setSyscallArg(int i, IntReg val, int tid)
{
    assert(i < TheISA::NumArgumentRegs);
    this->setArchIntReg(MipsISA::ArgumentReg[i], val, tid);
}

template <class Impl>
void
MipsO3CPU<Impl>::setSyscallReturn(SyscallReturn return_value, int tid)
{
    TheISA::setSyscallReturn(return_value, this->tcBase(tid));
}
#endif
