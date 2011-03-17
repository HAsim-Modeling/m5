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

#include "cpu/o3/mips/dyn_inst.hh"

template <class Impl>
MipsDynInst<Impl>::MipsDynInst(StaticInstPtr staticInst,
                               Addr PC, Addr NPC, Addr microPC,
                               Addr Pred_PC, Addr Pred_NPC, Addr Pred_MicroPC,
                               InstSeqNum seq_num, O3CPU *cpu)
    : BaseDynInst<Impl>(staticInst, PC, NPC, microPC,
            Pred_PC, Pred_NPC, Pred_MicroPC, seq_num, cpu)
{
    initVars();
}

template <class Impl>
MipsDynInst<Impl>::MipsDynInst(ExtMachInst inst,
                               Addr PC, Addr NPC, Addr microPC,
                               Addr Pred_PC, Addr Pred_NPC, Addr Pred_MicroPC,
                               InstSeqNum seq_num, O3CPU *cpu)
    : BaseDynInst<Impl>(inst, PC, NPC, microPC,
            Pred_PC, Pred_NPC, Pred_MicroPC, seq_num, cpu)
{
    initVars();
}

template <class Impl>
MipsDynInst<Impl>::MipsDynInst(StaticInstPtr &_staticInst)
    : BaseDynInst<Impl>(_staticInst)
{
    initVars();
}

template <class Impl>
void
MipsDynInst<Impl>::initVars()
{
    // Make sure to have the renamed register entries set to the same
    // as the normal register entries.  It will allow the IQ to work
    // without any modifications.
    for (int i = 0; i < this->staticInst->numDestRegs(); i++) {
        this->_destRegIdx[i] = this->staticInst->destRegIdx(i);
    }

    for (int i = 0; i < this->staticInst->numSrcRegs(); i++) {
        this->_srcRegIdx[i] = this->staticInst->srcRegIdx(i);
        this->_readySrcRegIdx[i] = 0;
    }
}

template <class Impl>
Fault
MipsDynInst<Impl>::execute()
{
    // @todo: Pretty convoluted way to avoid squashing from happening
    // when using the TC during an instruction's execution
    // (specifically for instructions that have side-effects that use
    // the TC).  Fix this.
    bool in_syscall = this->thread->inSyscall;
    this->thread->inSyscall = true;

    this->fault = this->staticInst->execute(this, this->traceData);

    this->thread->inSyscall = in_syscall;

    return this->fault;
}

template <class Impl>
Fault
MipsDynInst<Impl>::initiateAcc()
{
    // @todo: Pretty convoluted way to avoid squashing from happening
    // when using the TC during an instruction's execution
    // (specifically for instructions that have side-effects that use
    // the TC).  Fix this.
    bool in_syscall = this->thread->inSyscall;
    this->thread->inSyscall = true;

    this->fault = this->staticInst->initiateAcc(this, this->traceData);

    this->thread->inSyscall = in_syscall;

    return this->fault;
}

template <class Impl>
Fault
MipsDynInst<Impl>::completeAcc(PacketPtr pkt)
{
    this->fault = this->staticInst->completeAcc(pkt, this, this->traceData);

    return this->fault;
}

template <class Impl>
void
MipsDynInst<Impl>::syscall(int64_t callnum)
{
    this->cpu->syscall(callnum, this->threadNumber);
}

