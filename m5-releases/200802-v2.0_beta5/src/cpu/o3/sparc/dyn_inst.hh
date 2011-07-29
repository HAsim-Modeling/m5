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
 * Authors: Gabe M. Black
 */

#ifndef __CPU_O3_SPARC_DYN_INST_HH__
#define __CPU_O3_SPARC_DYN_INST_HH__

#include "arch/sparc/isa_traits.hh"
#include "arch/sparc/types.hh"
#include "cpu/base_dyn_inst.hh"
#include "cpu/inst_seq.hh"
#include "cpu/o3/sparc/cpu.hh"
#include "cpu/o3/sparc/impl.hh"

class Packet;

/**
 * Mostly implementation & ISA specific SparcDynInst. As with most
 * other classes in the new CPU model, it is templated on the Impl to
 * allow for passing in of all types, such as the CPU type and the ISA
 * type. The SparcDynInst serves as the primary interface to the CPU
 * for instructions that are executing.
 */
template <class Impl>
class SparcDynInst : public BaseDynInst<Impl>
{
  public:
    /** Typedef for the CPU. */
    typedef typename Impl::O3CPU O3CPU;

  public:
    /** BaseDynInst constructor given a binary instruction. */
    SparcDynInst(StaticInstPtr staticInst, Addr PC, Addr NPC, Addr microPC,
            Addr Pred_PC, Addr Pred_NPC, Addr Pred_MicroPC,
            InstSeqNum seq_num, O3CPU *cpu);

    /** BaseDynInst constructor given a binary instruction. */
    SparcDynInst(TheISA::ExtMachInst inst, Addr PC, Addr NPC, Addr microPC,
            Addr Pred_PC, Addr Pred_NPC, Addr Pred_MicroPC,
            InstSeqNum seq_num, O3CPU *cpu);

    /** BaseDynInst constructor given a static inst pointer. */
    SparcDynInst(StaticInstPtr &_staticInst);

    /** Executes the instruction.*/
    Fault execute();

    /** Initiates the access.  Only valid for memory operations. */
    Fault initiateAcc();

    /** Completes the access.  Only valid for memory operations. */
    Fault completeAcc(PacketPtr pkt);

  private:
    /** Initializes variables. */
    void initVars();

  public:
    /** Reads a miscellaneous register. */
    TheISA::MiscReg readMiscRegNoEffect(int misc_reg)
    {
        return this->cpu->readMiscRegNoEffect(misc_reg, this->threadNumber);
    }

    /** Reads a misc. register, including any side-effects the read
     * might have as defined by the architecture.
     */
    TheISA::MiscReg readMiscReg(int misc_reg)
    {
        return this->cpu->readMiscReg(misc_reg, this->threadNumber);
    }

    /** Sets a misc. register. */
    void setMiscRegNoEffect(int misc_reg, const TheISA::MiscReg &val)
    {
        this->instResult.integer = val;
        return this->cpu->setMiscRegNoEffect(misc_reg, val, this->threadNumber);
    }

    /** Sets a misc. register, including any side-effects the write
     * might have as defined by the architecture.
     */
    void setMiscReg(int misc_reg, const TheISA::MiscReg &val)
    {
        return this->cpu->setMiscReg(misc_reg, val,
                                               this->threadNumber);
    }

    /** Reads a miscellaneous register. */
    TheISA::MiscReg readMiscRegOperandNoEffect(const StaticInst *si, int idx)
    {
        return this->cpu->readMiscRegNoEffect(
                si->srcRegIdx(idx) - TheISA::Ctrl_Base_DepTag,
                this->threadNumber);
    }

    /** Reads a misc. register, including any side-effects the read
     * might have as defined by the architecture.
     */
    TheISA::MiscReg readMiscRegOperand(const StaticInst *si, int idx)
    {
        return this->cpu->readMiscReg(
                si->srcRegIdx(idx) - TheISA::Ctrl_Base_DepTag,
                this->threadNumber);
    }

    /** Sets a misc. register. */
    void setMiscRegOperandNoEffect(const StaticInst * si,
            int idx, const TheISA::MiscReg &val)
    {
        this->instResult.integer = val;
        return this->cpu->setMiscRegNoEffect(
                si->destRegIdx(idx) - TheISA::Ctrl_Base_DepTag,
                val, this->threadNumber);
    }

    /** Sets a misc. register, including any side-effects the write
     * might have as defined by the architecture.
     */
    void setMiscRegOperand(
            const StaticInst *si, int idx, const TheISA::MiscReg &val)
    {
        return this->cpu->setMiscReg(
                si->destRegIdx(idx) - TheISA::Ctrl_Base_DepTag,
                val, this->threadNumber);
    }

#if FULL_SYSTEM
    /** Calls hardware return from error interrupt. */
    Fault hwrei();
    /** Traps to handle specified fault. */
    void trap(Fault fault);
    bool simPalCheck(int palFunc);
#else
    /** Calls a syscall. */
    void syscall(int64_t callnum);
#endif

  public:

    // The register accessor methods provide the index of the
    // instruction's operand (e.g., 0 or 1), not the architectural
    // register index, to simplify the implementation of register
    // renaming.  We find the architectural register index by indexing
    // into the instruction's own operand index table.  Note that a
    // raw pointer to the StaticInst is provided instead of a
    // ref-counted StaticInstPtr to redice overhead.  This is fine as
    // long as these methods don't copy the pointer into any long-term
    // storage (which is pretty hard to imagine they would have reason
    // to do).

    uint64_t readIntRegOperand(const StaticInst *si, int idx)
    {
        uint64_t val = this->cpu->readIntReg(this->_srcRegIdx[idx]);
        DPRINTF(Sparc, "Reading int reg %d (%d, %d) as %x\n", (int)this->_flatSrcRegIdx[idx], (int)this->_srcRegIdx[idx], idx, val);
        return val;
    }

    TheISA::FloatReg readFloatRegOperand(const StaticInst *si,
            int idx, int width)
    {
        return this->cpu->readFloatReg(this->_srcRegIdx[idx], width);
    }

    TheISA::FloatReg readFloatRegOperand(const StaticInst *si, int idx)
    {
        return this->cpu->readFloatReg(this->_srcRegIdx[idx]);
    }

    TheISA::FloatRegBits readFloatRegOperandBits(const StaticInst *si,
            int idx, int width)
    {
        return this->cpu->readFloatRegBits(this->_srcRegIdx[idx], width);
    }

    TheISA::FloatRegBits readFloatRegOperandBits(const StaticInst *si, int idx)
    {
        return this->cpu->readFloatRegBits(this->_srcRegIdx[idx]);
    }

    /** @todo: Make results into arrays so they can handle multiple dest
     *  registers.
     */
    void setIntRegOperand(const StaticInst *si, int idx, uint64_t val)
    {
        DPRINTF(Sparc, "Setting int reg %d (%d, %d) to %x\n", (int)this->_flatDestRegIdx[idx], (int)this->_destRegIdx[idx], idx, val);
        this->cpu->setIntReg(this->_destRegIdx[idx], val);
        BaseDynInst<Impl>::setIntRegOperand(si, idx, val);
    }

    void setFloatRegOperand(const StaticInst *si, int idx,
            TheISA::FloatReg val, int width)
    {
        this->cpu->setFloatReg(this->_destRegIdx[idx], val, width);
        BaseDynInst<Impl>::setFloatRegOperand(si, idx, val, width);
    }

    void setFloatRegOperand(const StaticInst *si, int idx, TheISA::FloatReg val)
    {
        this->cpu->setFloatReg(this->_destRegIdx[idx], val);
        BaseDynInst<Impl>::setFloatRegOperand(si, idx, val);
    }

    void setFloatRegOperandBits(const StaticInst *si, int idx,
            TheISA::FloatRegBits val, int width)
    {
        this->cpu->setFloatRegBits(this->_destRegIdx[idx], val, width);
        BaseDynInst<Impl>::setFloatRegOperandBits(si, idx, val);
    }

    void setFloatRegOperandBits(const StaticInst *si,
            int idx, TheISA::FloatRegBits val)
    {
        this->cpu->setFloatRegBits(this->_destRegIdx[idx], val);
        BaseDynInst<Impl>::setFloatRegOperandBits(si, idx, val);
    }

  public:
    /** Calculates EA part of a memory instruction. Currently unused,
     * though it may be useful in the future if we want to split
     * memory operations into EA calculation and memory access parts.
     */
    Fault calcEA()
    {
        return this->staticInst->eaCompInst()->execute(this, this->traceData);
    }

    /** Does the memory access part of a memory instruction. Currently unused,
     * though it may be useful in the future if we want to split
     * memory operations into EA calculation and memory access parts.
     */
    Fault memAccess()
    {
        return this->staticInst->memAccInst()->execute(this, this->traceData);
    }
};

#endif // __CPU_O3_SPARC_DYN_INST_HH__
