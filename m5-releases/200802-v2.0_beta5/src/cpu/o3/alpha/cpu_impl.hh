/*
 * Copyright (c) 2004, 2005, 2006
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
 */

#include "config/use_checker.hh"

#include "arch/alpha/faults.hh"
#include "arch/alpha/isa_traits.hh"
#include "base/cprintf.hh"
#include "base/statistics.hh"
#include "base/timebuf.hh"
#include "cpu/checker/thread_context.hh"
#include "sim/sim_events.hh"
#include "sim/stats.hh"

#include "cpu/o3/alpha/cpu.hh"
#include "cpu/o3/alpha/params.hh"
#include "cpu/o3/alpha/thread_context.hh"
#include "cpu/o3/comm.hh"
#include "cpu/o3/thread_state.hh"

#if FULL_SYSTEM
#include "arch/alpha/osfpal.hh"
#include "arch/isa_traits.hh"
#include "arch/kernel_stats.hh"
#include "cpu/quiesce_event.hh"
#include "sim/sim_exit.hh"
#include "sim/system.hh"
#endif

template <class Impl>
AlphaO3CPU<Impl>::AlphaO3CPU(Params *params) : FullO3CPU<Impl>(this, params)
{
    DPRINTF(O3CPU, "Creating AlphaO3CPU object.\n");

    // Setup any thread state.
    this->thread.resize(this->numThreads);

    for (int i = 0; i < this->numThreads; ++i) {
#if FULL_SYSTEM
        // SMT is not supported in FS mode yet.
        assert(this->numThreads == 1);
        this->thread[i] = new Thread(this, 0);
        this->thread[i]->setStatus(ThreadContext::Suspended);
#else
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
#endif // !FULL_SYSTEM

        ThreadContext *tc;

        // Setup the TC that will serve as the interface to the threads/CPU.
        AlphaTC<Impl> *alpha_tc =
            new AlphaTC<Impl>;

        tc = alpha_tc;

        // If we're using a checker, then the TC should be the
        // CheckerThreadContext.
#if USE_CHECKER
        if (params->checker) {
            tc = new CheckerThreadContext<AlphaTC<Impl> >(
                alpha_tc, this->checker);
        }
#endif

        alpha_tc->cpu = this;
        alpha_tc->thread = this->thread[i];

#if FULL_SYSTEM
        // Setup quiesce event.
        this->thread[i]->quiesceEvent = new EndQuiesceEvent(tc);
#endif
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
AlphaO3CPU<Impl>::regStats()
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
TheISA::MiscReg
AlphaO3CPU<Impl>::readMiscRegNoEffect(int misc_reg, unsigned tid)
{
    return this->regFile.readMiscRegNoEffect(misc_reg, tid);
}

template <class Impl>
TheISA::MiscReg
AlphaO3CPU<Impl>::readMiscReg(int misc_reg, unsigned tid)
{
    return this->regFile.readMiscReg(misc_reg, tid);
}

template <class Impl>
void
AlphaO3CPU<Impl>::setMiscRegNoEffect(int misc_reg, const TheISA::MiscReg &val,
        unsigned tid)
{
    this->regFile.setMiscRegNoEffect(misc_reg, val, tid);
}

template <class Impl>
void
AlphaO3CPU<Impl>::setMiscReg(int misc_reg,
        const TheISA::MiscReg &val, unsigned tid)
{
    this->regFile.setMiscReg(misc_reg, val, tid);
}

template <class Impl>
void
AlphaO3CPU<Impl>::squashFromTC(unsigned tid)
{
    this->thread[tid]->inSyscall = true;
    this->commit.generateTCEvent(tid);
}

#if FULL_SYSTEM

template <class Impl>
void
AlphaO3CPU<Impl>::post_interrupt(int int_num, int index)
{
    BaseCPU::post_interrupt(int_num, index);

    if (this->thread[0]->status() == ThreadContext::Suspended) {
        DPRINTF(IPI,"Suspended Processor awoke\n");
        this->threadContexts[0]->activate();
    }
}

template <class Impl>
Fault
AlphaO3CPU<Impl>::hwrei(unsigned tid)
{
    // Need to clear the lock flag upon returning from an interrupt.
    this->setMiscRegNoEffect(AlphaISA::MISCREG_LOCKFLAG, false, tid);

    this->thread[tid]->kernelStats->hwrei();

    // FIXME: XXX check for interrupts? XXX
    return NoFault;
}

template <class Impl>
bool
AlphaO3CPU<Impl>::simPalCheck(int palFunc, unsigned tid)
{
    if (this->thread[tid]->kernelStats)
        this->thread[tid]->kernelStats->callpal(palFunc,
                                                this->threadContexts[tid]);

    switch (palFunc) {
      case PAL::halt:
        halt();
        if (--System::numSystemsRunning == 0)
            exitSimLoop("all cpus halted");
        break;

      case PAL::bpt:
      case PAL::bugchk:
        if (this->system->breakpoint())
            return false;
        break;
    }

    return true;
}

template <class Impl>
Fault
AlphaO3CPU<Impl>::getInterrupts()
{
    // Check if there are any outstanding interrupts
    return this->interrupts.getInterrupt(this->threadContexts[0]);
}

template <class Impl>
void
AlphaO3CPU<Impl>::processInterrupts(Fault interrupt)
{
    // Check for interrupts here.  For now can copy the code that
    // exists within isa_fullsys_traits.hh.  Also assume that thread 0
    // is the one that handles the interrupts.
    // @todo: Possibly consolidate the interrupt checking code.
    // @todo: Allow other threads to handle interrupts.

    assert(interrupt != NoFault);
    this->interrupts.updateIntrInfo(this->threadContexts[0]);

    DPRINTF(O3CPU, "Interrupt %s being handled\n", interrupt->name());
    this->trap(interrupt, 0);
}

#endif // FULL_SYSTEM

template <class Impl>
void
AlphaO3CPU<Impl>::trap(Fault fault, unsigned tid)
{
    // Pass the thread's TC into the invoke method.
    fault->invoke(this->threadContexts[tid]);
}

#if !FULL_SYSTEM

template <class Impl>
void
AlphaO3CPU<Impl>::syscall(int64_t callnum, int tid)
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
}

template <class Impl>
TheISA::IntReg
AlphaO3CPU<Impl>::getSyscallArg(int i, int tid)
{
    assert(i < TheISA::NumArgumentRegs);
    return this->readArchIntReg(AlphaISA::ArgumentReg[i], tid);
}

template <class Impl>
void
AlphaO3CPU<Impl>::setSyscallArg(int i, TheISA::IntReg val, int tid)
{
    assert(i < TheISA::NumArgumentRegs);
    this->setArchIntReg(AlphaISA::ArgumentReg[i], val, tid);
}

template <class Impl>
void
AlphaO3CPU<Impl>::setSyscallReturn(SyscallReturn return_value, int tid)
{
    TheISA::setSyscallReturn(return_value, this->tcBase(tid));
}
#endif
