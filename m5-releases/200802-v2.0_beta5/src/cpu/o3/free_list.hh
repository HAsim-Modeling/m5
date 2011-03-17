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
 * Authors: Kevin T. Lim
 */

#ifndef __CPU_O3_FREE_LIST_HH__
#define __CPU_O3_FREE_LIST_HH__

#include <iostream>
#include <queue>

#include "arch/isa_traits.hh"
#include "base/misc.hh"
#include "base/trace.hh"
#include "base/traceflags.hh"
#include "cpu/o3/comm.hh"

/**
 * FreeList class that simply holds the list of free integer and floating
 * point registers.  Can request for a free register of either type, and
 * also send back free registers of either type.  This is a very simple
 * class, but it should be sufficient for most implementations.  Like all
 * other classes, it assumes that the indices for the floating point
 * registers starts after the integer registers end.  Hence the variable
 * numPhysicalIntRegs is logically equivalent to the baseFP dependency.
 * Note that while this most likely should be called FreeList, the name
 * "FreeList" is used in a typedef within the CPU Policy, and therefore no
 * class can be named simply "FreeList".
 * @todo: Give a better name to the base FP dependency.
 */
class SimpleFreeList
{
  private:
    /** The list of free integer registers. */
    std::queue<PhysRegIndex> freeIntRegs;

    /** The list of free floating point registers. */
    std::queue<PhysRegIndex> freeFloatRegs;

    /** Number of logical integer registers. */
    int numLogicalIntRegs;

    /** Number of physical integer registers. */
    int numPhysicalIntRegs;

    /** Number of logical floating point registers. */
    int numLogicalFloatRegs;

    /** Number of physical floating point registers. */
    int numPhysicalFloatRegs;

    /** Total number of physical registers. */
    int numPhysicalRegs;

  public:
    /** Constructs a free list.
     *  @param activeThreads Number of active threads.
     *  @param _numLogicalIntRegs Number of logical integer registers.
     *  @param _numPhysicalIntRegs Number of physical integer registers.
     *  @param _numLogicalFloatRegs Number of logical fp registers.
     *  @param _numPhysicalFloatRegs Number of physical fp registers.
     */
    SimpleFreeList(unsigned activeThreads,
                   unsigned _numLogicalIntRegs,
                   unsigned _numPhysicalIntRegs,
                   unsigned _numLogicalFloatRegs,
                   unsigned _numPhysicalFloatRegs);

    /** Gives the name of the freelist. */
    std::string name() const;

    /** Gets a free integer register. */
    inline PhysRegIndex getIntReg();

    /** Gets a free fp register. */
    inline PhysRegIndex getFloatReg();

    /** Adds a register back to the free list. */
    inline void addReg(PhysRegIndex freed_reg);

    /** Adds an integer register back to the free list. */
    inline void addIntReg(PhysRegIndex freed_reg);

    /** Adds a fp register back to the free list. */
    inline void addFloatReg(PhysRegIndex freed_reg);

    /** Checks if there are any free integer registers. */
    bool hasFreeIntRegs()
    { return !freeIntRegs.empty(); }

    /** Checks if there are any free fp registers. */
    bool hasFreeFloatRegs()
    { return !freeFloatRegs.empty(); }

    /** Returns the number of free integer registers. */
    int numFreeIntRegs()
    { return freeIntRegs.size(); }

    /** Returns the number of free fp registers. */
    int numFreeFloatRegs()
    { return freeFloatRegs.size(); }
};

inline PhysRegIndex
SimpleFreeList::getIntReg()
{
    DPRINTF(FreeList, "Trying to get free integer register.\n");

    if (freeIntRegs.empty()) {
        panic("No free integer registers!");
    }

    PhysRegIndex free_reg = freeIntRegs.front();

    freeIntRegs.pop();

    return(free_reg);
}

inline PhysRegIndex
SimpleFreeList::getFloatReg()
{
    DPRINTF(FreeList, "Trying to get free float register.\n");

    if (freeFloatRegs.empty()) {
        panic("No free integer registers!");
    }

    PhysRegIndex free_reg = freeFloatRegs.front();

    freeFloatRegs.pop();

    return(free_reg);
}

inline void
SimpleFreeList::addReg(PhysRegIndex freed_reg)
{
    DPRINTF(FreeList,"Freeing register %i.\n", freed_reg);
    //Might want to add in a check for whether or not this register is
    //already in there.  A bit vector or something similar would be useful.
    if (freed_reg < numPhysicalIntRegs) {
        if (freed_reg != TheISA::ZeroReg)
            freeIntRegs.push(freed_reg);
    } else if (freed_reg < numPhysicalRegs) {
#if THE_ISA == ALPHA_ISA
        if (freed_reg != (TheISA::ZeroReg + numPhysicalIntRegs))
#endif
            freeFloatRegs.push(freed_reg);
    }

    // These assert conditions ensure that the number of free 
    // registers are not more than the # of total Physical  Registers.
    // If this were false, it would mean that registers
    // have been freed twice, overflowing the free register
    // pool and potentially crashing SMT workloads.
    // ----
    // Comment out for now so as to not potentially break
    // CMP and single-threaded workloads
    // ----
    // assert(freeIntRegs.size() <= numPhysicalIntRegs);
    // assert(freeFloatRegs.size() <= numPhysicalFloatRegs);
}

inline void
SimpleFreeList::addIntReg(PhysRegIndex freed_reg)
{
    DPRINTF(FreeList,"Freeing int register %i.\n", freed_reg);

    freeIntRegs.push(freed_reg);
}

inline void
SimpleFreeList::addFloatReg(PhysRegIndex freed_reg)
{
    DPRINTF(FreeList,"Freeing float register %i.\n", freed_reg);

    freeFloatRegs.push(freed_reg);
}

#endif // __CPU_O3_FREE_LIST_HH__
