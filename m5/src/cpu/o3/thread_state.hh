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
 */

#ifndef __CPU_O3_THREAD_STATE_HH__
#define __CPU_O3_THREAD_STATE_HH__

#include "base/callback.hh"
#include "base/output.hh"
#include "cpu/thread_context.hh"
#include "cpu/thread_state.hh"
#include "sim/sim_exit.hh"

class Event;
class Process;

#if FULL_SYSTEM
class EndQuiesceEvent;
class FunctionProfile;
class ProfileNode;
#else
class FunctionalMemory;
class Process;
#endif

/**
 * Class that has various thread state, such as the status, the
 * current instruction being processed, whether or not the thread has
 * a trap pending or is being externally updated, the ThreadContext
 * pointer, etc.  It also handles anything related to a specific
 * thread's process, such as syscalls and checking valid addresses.
 */
template <class Impl>
struct O3ThreadState : public ThreadState {
    typedef ThreadContext::Status Status;
    typedef typename Impl::O3CPU O3CPU;

  private:
    /** Pointer to the CPU. */
    O3CPU *cpu;
  public:
    /** Whether or not the thread is currently in syscall mode, and
     * thus able to be externally updated without squashing.
     */
    bool inSyscall;

    /** Whether or not the thread is currently waiting on a trap, and
     * thus able to be externally updated without squashing.
     */
    bool trapPending;

#if FULL_SYSTEM
    O3ThreadState(O3CPU *_cpu, int _thread_num)
        : ThreadState(_cpu, -1, _thread_num),
          cpu(_cpu), inSyscall(0), trapPending(0)
    {
        if (cpu->params->profile) {
            profile = new FunctionProfile(cpu->params->system->kernelSymtab);
            Callback *cb =
                new MakeCallback<O3ThreadState,
                &O3ThreadState::dumpFuncProfile>(this);
            registerExitCallback(cb);
        }

        // let's fill with a dummy node for now so we don't get a segfault
        // on the first cycle when there's no node available.
        static ProfileNode dummyNode;
        profileNode = &dummyNode;
        profilePC = 3;
    }
#else
    O3ThreadState(O3CPU *_cpu, int _thread_num, Process *_process, int _asid)
        : ThreadState(_cpu, -1, _thread_num, _process, _asid),
          cpu(_cpu), inSyscall(0), trapPending(0)
    { }
#endif

    /** Pointer to the ThreadContext of this thread. */
    ThreadContext *tc;

    /** Returns a pointer to the TC of this thread. */
    ThreadContext *getTC() { return tc; }

#if !FULL_SYSTEM
    /** Handles the syscall. */
    void syscall(int64_t callnum) { process->syscall(callnum, tc); }
#endif

#if FULL_SYSTEM
    void dumpFuncProfile()
    {
        std::ostream *os = simout.create(csprintf("profile.%s.dat", cpu->name()));
        profile->dump(tc, *os);
    }
#endif
};

#endif // __CPU_O3_THREAD_STATE_HH__
