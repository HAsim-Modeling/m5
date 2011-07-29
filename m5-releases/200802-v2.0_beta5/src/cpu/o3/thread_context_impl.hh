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
 *          Korey L. Sewell
 */

#include "arch/regfile.hh"
#include "cpu/o3/thread_context.hh"
#include "cpu/quiesce_event.hh"

#if FULL_SYSTEM
template <class Impl>
VirtualPort *
O3ThreadContext<Impl>::getVirtPort(ThreadContext *src_tc)
{
    if (!src_tc)
        return thread->getVirtPort();

    VirtualPort *vp;

    vp = new VirtualPort("tc-vport", src_tc);
    thread->connectToMemFunc(vp);
    return vp;
}

template <class Impl>
void
O3ThreadContext<Impl>::dumpFuncProfile()
{
    thread->dumpFuncProfile();
}
#endif

template <class Impl>
void
O3ThreadContext<Impl>::takeOverFrom(ThreadContext *old_context)
{
    // some things should already be set up
#if FULL_SYSTEM
    assert(getSystemPtr() == old_context->getSystemPtr());
#else
    assert(getProcessPtr() == old_context->getProcessPtr());
#endif

    // copy over functional state
    setStatus(old_context->status());
    copyArchRegs(old_context);
    setCpuId(old_context->readCpuId());

#if !FULL_SYSTEM
    thread->funcExeInst = old_context->readFuncExeInst();
#else
    EndQuiesceEvent *other_quiesce = old_context->getQuiesceEvent();
    if (other_quiesce) {
        // Point the quiesce event's TC at this TC so that it wakes up
        // the proper CPU.
        other_quiesce->tc = this;
    }
    if (thread->quiesceEvent) {
        thread->quiesceEvent->tc = this;
    }

    // Transfer kernel stats from one CPU to the other.
    thread->kernelStats = old_context->getKernelStats();
//    storeCondFailures = 0;
    cpu->lockFlag = false;
#endif

    old_context->setStatus(ThreadContext::Unallocated);

    thread->inSyscall = false;
    thread->trapPending = false;
}

#if FULL_SYSTEM
template <class Impl>
void
O3ThreadContext<Impl>::delVirtPort(VirtualPort *vp)
{
    if (vp != thread->getVirtPort()) {
        vp->removeConn();
        delete vp;
    }
}
#endif

template <class Impl>
void
O3ThreadContext<Impl>::activate(int delay)
{
    DPRINTF(O3CPU, "Calling activate on Thread Context %d\n",
            getThreadNum());

    if (thread->status() == ThreadContext::Active)
        return;

#if FULL_SYSTEM
    thread->lastActivate = curTick;
#endif

    if (thread->status() == ThreadContext::Unallocated) {
        cpu->activateWhenReady(thread->readTid());
        return;
    }

    thread->setStatus(ThreadContext::Active);

    // status() == Suspended
    cpu->activateContext(thread->readTid(), delay);
}

template <class Impl>
void
O3ThreadContext<Impl>::suspend(int delay)
{
    DPRINTF(O3CPU, "Calling suspend on Thread Context %d\n",
            getThreadNum());

    if (thread->status() == ThreadContext::Suspended)
        return;

#if FULL_SYSTEM
    thread->lastActivate = curTick;
    thread->lastSuspend = curTick;
#endif
/*
#if FULL_SYSTEM
    // Don't change the status from active if there are pending interrupts
    if (cpu->check_interrupts()) {
        assert(status() == ThreadContext::Active);
        return;
    }
#endif
*/
    thread->setStatus(ThreadContext::Suspended);
    cpu->suspendContext(thread->readTid());
}

template <class Impl>
void
O3ThreadContext<Impl>::deallocate(int delay)
{
    DPRINTF(O3CPU, "Calling deallocate on Thread Context %d delay %d\n",
            getThreadNum(), delay);

    if (thread->status() == ThreadContext::Unallocated)
        return;

    thread->setStatus(ThreadContext::Unallocated);
    cpu->deallocateContext(thread->readTid(), true, delay);
}

template <class Impl>
void
O3ThreadContext<Impl>::halt(int delay)
{
    DPRINTF(O3CPU, "Calling halt on Thread Context %d\n",
            getThreadNum());

    if (thread->status() == ThreadContext::Halted)
        return;

    thread->setStatus(ThreadContext::Halted);
    cpu->haltContext(thread->readTid());
}

template <class Impl>
void
O3ThreadContext<Impl>::regStats(const std::string &name)
{
#if FULL_SYSTEM
    thread->kernelStats = new TheISA::Kernel::Statistics(cpu->system);
    thread->kernelStats->regStats(name + ".kern");
#endif
}

template <class Impl>
void
O3ThreadContext<Impl>::serialize(std::ostream &os)
{
#if FULL_SYSTEM
    if (thread->kernelStats)
        thread->kernelStats->serialize(os);
#endif

}

template <class Impl>
void
O3ThreadContext<Impl>::unserialize(Checkpoint *cp, const std::string &section)
{
#if FULL_SYSTEM
    if (thread->kernelStats)
        thread->kernelStats->unserialize(cp, section);
#endif

}

#if FULL_SYSTEM
template <class Impl>
Tick
O3ThreadContext<Impl>::readLastActivate()
{
    return thread->lastActivate;
}

template <class Impl>
Tick
O3ThreadContext<Impl>::readLastSuspend()
{
    return thread->lastSuspend;
}

template <class Impl>
void
O3ThreadContext<Impl>::profileClear()
{
    thread->profileClear();
}

template <class Impl>
void
O3ThreadContext<Impl>::profileSample()
{
    thread->profileSample();
}
#endif

template <class Impl>
TheISA::MachInst
O3ThreadContext<Impl>:: getInst()
{
    return thread->getInst();
}

template <class Impl>
void
O3ThreadContext<Impl>::copyArchRegs(ThreadContext *tc)
{
    // This function will mess things up unless the ROB is empty and
    // there are no instructions in the pipeline.
    unsigned tid = thread->readTid();
    PhysRegIndex renamed_reg;

    // First loop through the integer registers.
    for (int i = 0; i < TheISA::NumIntRegs; ++i) {
        renamed_reg = cpu->renameMap[tid].lookup(i);

        DPRINTF(O3CPU, "Copying over register %i, had data %lli, "
                "now has data %lli.\n",
                renamed_reg, cpu->readIntReg(renamed_reg),
                tc->readIntReg(i));

        cpu->setIntReg(renamed_reg, tc->readIntReg(i));
    }

    // Then loop through the floating point registers.
    for (int i = 0; i < TheISA::NumFloatRegs; ++i) {
        renamed_reg = cpu->renameMap[tid].lookup(i + TheISA::FP_Base_DepTag);
        cpu->setFloatRegBits(renamed_reg,
                             tc->readFloatRegBits(i));
    }

    // Copy the misc regs.
    TheISA::copyMiscRegs(tc, this);

    // Then finally set the PC, the next PC, the nextNPC, the micropc, and the
    // next micropc.
    cpu->setPC(tc->readPC(), tid);
    cpu->setNextPC(tc->readNextPC(), tid);
    cpu->setNextNPC(tc->readNextNPC(), tid);
    cpu->setMicroPC(tc->readMicroPC(), tid);
    cpu->setNextMicroPC(tc->readNextMicroPC(), tid);
#if !FULL_SYSTEM
    this->thread->funcExeInst = tc->readFuncExeInst();
#endif
}

template <class Impl>
void
O3ThreadContext<Impl>::clearArchRegs()
{}

template <class Impl>
uint64_t
O3ThreadContext<Impl>::readIntReg(int reg_idx)
{
    reg_idx = TheISA::flattenIntIndex(this, reg_idx);
    return cpu->readArchIntReg(reg_idx, thread->readTid());
}

template <class Impl>
TheISA::FloatReg
O3ThreadContext<Impl>::readFloatReg(int reg_idx, int width)
{
    reg_idx = TheISA::flattenFloatIndex(this, reg_idx);
    switch(width) {
      case 32:
        return cpu->readArchFloatRegSingle(reg_idx, thread->readTid());
      case 64:
        return cpu->readArchFloatRegDouble(reg_idx, thread->readTid());
      default:
        panic("Unsupported width!");
        return 0;
    }
}

template <class Impl>
TheISA::FloatReg
O3ThreadContext<Impl>::readFloatReg(int reg_idx)
{
    reg_idx = TheISA::flattenFloatIndex(this, reg_idx);
    return cpu->readArchFloatRegSingle(reg_idx, thread->readTid());
}

template <class Impl>
TheISA::FloatRegBits
O3ThreadContext<Impl>::readFloatRegBits(int reg_idx, int width)
{
    DPRINTF(Fault, "Reading floatint register through the TC!\n");
    reg_idx = TheISA::flattenFloatIndex(this, reg_idx);
    return cpu->readArchFloatRegInt(reg_idx, thread->readTid());
}

template <class Impl>
TheISA::FloatRegBits
O3ThreadContext<Impl>::readFloatRegBits(int reg_idx)
{
    reg_idx = TheISA::flattenFloatIndex(this, reg_idx);
    return cpu->readArchFloatRegInt(reg_idx, thread->readTid());
}

template <class Impl>
void
O3ThreadContext<Impl>::setIntReg(int reg_idx, uint64_t val)
{
    reg_idx = TheISA::flattenIntIndex(this, reg_idx);
    cpu->setArchIntReg(reg_idx, val, thread->readTid());

    // Squash if we're not already in a state update mode.
    if (!thread->trapPending && !thread->inSyscall) {
        cpu->squashFromTC(thread->readTid());
    }
}

template <class Impl>
void
O3ThreadContext<Impl>::setFloatReg(int reg_idx, FloatReg val, int width)
{
    reg_idx = TheISA::flattenFloatIndex(this, reg_idx);
    switch(width) {
      case 32:
        cpu->setArchFloatRegSingle(reg_idx, val, thread->readTid());
        break;
      case 64:
        cpu->setArchFloatRegDouble(reg_idx, val, thread->readTid());
        break;
    }

    // Squash if we're not already in a state update mode.
    if (!thread->trapPending && !thread->inSyscall) {
        cpu->squashFromTC(thread->readTid());
    }
}

template <class Impl>
void
O3ThreadContext<Impl>::setFloatReg(int reg_idx, FloatReg val)
{
    reg_idx = TheISA::flattenFloatIndex(this, reg_idx);
    cpu->setArchFloatRegSingle(reg_idx, val, thread->readTid());

    if (!thread->trapPending && !thread->inSyscall) {
        cpu->squashFromTC(thread->readTid());
    }
}

template <class Impl>
void
O3ThreadContext<Impl>::setFloatRegBits(int reg_idx, FloatRegBits val,
                                             int width)
{
    DPRINTF(Fault, "Setting floatint register through the TC!\n");
    reg_idx = TheISA::flattenFloatIndex(this, reg_idx);
    cpu->setArchFloatRegInt(reg_idx, val, thread->readTid());

    // Squash if we're not already in a state update mode.
    if (!thread->trapPending && !thread->inSyscall) {
        cpu->squashFromTC(thread->readTid());
    }
}

template <class Impl>
void
O3ThreadContext<Impl>::setFloatRegBits(int reg_idx, FloatRegBits val)
{
    reg_idx = TheISA::flattenFloatIndex(this, reg_idx);
    cpu->setArchFloatRegInt(reg_idx, val, thread->readTid());

    // Squash if we're not already in a state update mode.
    if (!thread->trapPending && !thread->inSyscall) {
        cpu->squashFromTC(thread->readTid());
    }
}

template <class Impl>
void
O3ThreadContext<Impl>::setPC(uint64_t val)
{
    cpu->setPC(val, thread->readTid());

    // Squash if we're not already in a state update mode.
    if (!thread->trapPending && !thread->inSyscall) {
        cpu->squashFromTC(thread->readTid());
    }
}

template <class Impl>
void
O3ThreadContext<Impl>::setNextPC(uint64_t val)
{
    cpu->setNextPC(val, thread->readTid());

    // Squash if we're not already in a state update mode.
    if (!thread->trapPending && !thread->inSyscall) {
        cpu->squashFromTC(thread->readTid());
    }
}

template <class Impl>
void
O3ThreadContext<Impl>::setMicroPC(uint64_t val)
{
    cpu->setMicroPC(val, thread->readTid());

    // Squash if we're not already in a state update mode.
    if (!thread->trapPending && !thread->inSyscall) {
        cpu->squashFromTC(thread->readTid());
    }
}

template <class Impl>
void
O3ThreadContext<Impl>::setNextMicroPC(uint64_t val)
{
    cpu->setNextMicroPC(val, thread->readTid());

    // Squash if we're not already in a state update mode.
    if (!thread->trapPending && !thread->inSyscall) {
        cpu->squashFromTC(thread->readTid());
    }
}

template <class Impl>
void
O3ThreadContext<Impl>::setMiscRegNoEffect(int misc_reg, const MiscReg &val)
{
    cpu->setMiscRegNoEffect(misc_reg, val, thread->readTid());

    // Squash if we're not already in a state update mode.
    if (!thread->trapPending && !thread->inSyscall) {
        cpu->squashFromTC(thread->readTid());
    }
}

template <class Impl>
void
O3ThreadContext<Impl>::setMiscReg(int misc_reg,
                                                const MiscReg &val)
{
    cpu->setMiscReg(misc_reg, val, thread->readTid());

    // Squash if we're not already in a state update mode.
    if (!thread->trapPending && !thread->inSyscall) {
        cpu->squashFromTC(thread->readTid());
    }
}

#if !FULL_SYSTEM

template <class Impl>
TheISA::IntReg
O3ThreadContext<Impl>::getSyscallArg(int i)
{
    return cpu->getSyscallArg(i, thread->readTid());
}

template <class Impl>
void
O3ThreadContext<Impl>::setSyscallArg(int i, IntReg val)
{
    cpu->setSyscallArg(i, val, thread->readTid());
}

template <class Impl>
void
O3ThreadContext<Impl>::setSyscallReturn(SyscallReturn return_value)
{
    cpu->setSyscallReturn(return_value, thread->readTid());
}

#endif // FULL_SYSTEM
