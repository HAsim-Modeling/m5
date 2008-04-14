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

#ifndef __CPU_O3_ALPHA_CPU_HH__
#define __CPU_O3_ALPHA_CPU_HH__

#include "arch/regfile.hh"
#include "arch/types.hh"
#include "cpu/thread_context.hh"
#include "cpu/o3/cpu.hh"
#include "sim/byteswap.hh"

class EndQuiesceEvent;
namespace Kernel {
    class Statistics;
};

class TranslatingPort;

/**
 * AlphaO3CPU class.  Derives from the FullO3CPU class, and
 * implements all ISA and implementation specific functions of the
 * CPU.  This is the CPU class that is used for the SimObjects, and is
 * what is given to the DynInsts.  Most of its state exists in the
 * FullO3CPU; the state is has is mainly for ISA specific
 * functionality.
 */
template <class Impl>
class AlphaO3CPU : public FullO3CPU<Impl>
{
  public:
    typedef O3ThreadState<Impl> ImplState;
    typedef O3ThreadState<Impl> Thread;
    typedef typename Impl::Params Params;

    /** Constructs an AlphaO3CPU with the given parameters. */
    AlphaO3CPU(Params *params);

    /** Registers statistics. */
    void regStats();

    /** Reads a miscellaneous register. */
    TheISA::MiscReg readMiscRegNoEffect(int misc_reg, unsigned tid);

    /** Reads a misc. register, including any side effects the read
     * might have as defined by the architecture.
     */
    TheISA::MiscReg readMiscReg(int misc_reg, unsigned tid);

    /** Sets a miscellaneous register. */
    void setMiscRegNoEffect(int misc_reg, const TheISA::MiscReg &val,
            unsigned tid);

    /** Sets a misc. register, including any side effects the write
     * might have as defined by the architecture.
     */
    void setMiscReg(int misc_reg, const TheISA::MiscReg &val,
            unsigned tid);

    /** Initiates a squash of all in-flight instructions for a given
     * thread.  The source of the squash is an external update of
     * state through the TC.
     */
    void squashFromTC(unsigned tid);

#if FULL_SYSTEM
    /** Posts an interrupt. */
    void post_interrupt(int int_num, int index);
    /** HW return from error interrupt. */
    Fault hwrei(unsigned tid);

    bool simPalCheck(int palFunc, unsigned tid);

    /** Returns the Fault for any valid interrupt. */
    Fault getInterrupts();

    /** Processes any an interrupt fault. */
    void processInterrupts(Fault interrupt);

    /** Halts the CPU. */
    void halt() { panic("Halt not implemented!\n"); }
#endif

    /** Traps to handle given fault. */
    void trap(Fault fault, unsigned tid);

#if !FULL_SYSTEM
    /** Executes a syscall.
     * @todo: Determine if this needs to be virtual.
     */
    void syscall(int64_t callnum, int tid);
    /** Gets a syscall argument. */
    TheISA::IntReg getSyscallArg(int i, int tid);

    /** Used to shift args for indirect syscall. */
    void setSyscallArg(int i, TheISA::IntReg val, int tid);

    /** Sets the return value of a syscall. */
    void setSyscallReturn(SyscallReturn return_value, int tid);
#endif

    /** CPU read function, forwards read to LSQ. */
    template <class T>
    Fault read(RequestPtr &req, T &data, int load_idx)
    {
        return this->iew.ldstQueue.read(req, data, load_idx);
    }

    /** CPU write function, forwards write to LSQ. */
    template <class T>
    Fault write(RequestPtr &req, T &data, int store_idx)
    {
        return this->iew.ldstQueue.write(req, data, store_idx);
    }

    Addr lockAddr;

    /** Temporary fix for the lock flag, works in the UP case. */
    bool lockFlag;
};

#endif // __CPU_O3_ALPHA_CPU_HH__
