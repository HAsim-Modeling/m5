/*
 * Copyright (c) 2005, 2006
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
 * Authors: Korey L. Sewell
 *          Kevin T. Lim
 */

#ifndef __CPU_O3_SCOREBOARD_HH__
#define __CPU_O3_SCOREBOARD_HH__

#include <iostream>
#include <utility>
#include <vector>
#include "base/trace.hh"
#include "base/traceflags.hh"
#include "cpu/o3/comm.hh"

/**
 * Implements a simple scoreboard to track which registers are ready.
 * This class assumes that the fp registers start, index wise, right after
 * the integer registers. The misc. registers start, index wise, right after
 * the fp registers.
 * @todo: Fix up handling of the zero register in case the decoder does not
 * automatically make insts that write the zero register into nops.
 */
class Scoreboard
{
  public:
    /** Constructs a scoreboard.
     *  @param activeThreads The number of active threads.
     *  @param _numLogicalIntRegs Number of logical integer registers.
     *  @param _numPhysicalIntRegs Number of physical integer registers.
     *  @param _numLogicalFloatRegs Number of logical fp registers.
     *  @param _numPhysicalFloatRegs Number of physical fp registers.
     *  @param _numMiscRegs Number of miscellaneous registers.
     *  @param _zeroRegIdx Index of the zero register.
     */
    Scoreboard(unsigned activeThreads,
               unsigned _numLogicalIntRegs,
               unsigned _numPhysicalIntRegs,
               unsigned _numLogicalFloatRegs,
               unsigned _numPhysicalFloatRegs,
               unsigned _numMiscRegs,
               unsigned _zeroRegIdx);

    /** Destructor. */
    ~Scoreboard() {}

    /** Returns the name of the scoreboard. */
    std::string name() const;

    /** Checks if the register is ready. */
    bool getReg(PhysRegIndex ready_reg);

    /** Sets the register as ready. */
    void setReg(PhysRegIndex phys_reg);

    /** Sets the register as not ready. */
    void unsetReg(PhysRegIndex ready_reg);

  private:
    /** Scoreboard of physical integer registers, saying whether or not they
     *  are ready.
     */
    std::vector<bool> regScoreBoard;

    /** Number of logical integer registers. */
    int numLogicalIntRegs;

    /** Number of physical integer registers. */
    int numPhysicalIntRegs;

    /** Number of logical floating point registers. */
    int numLogicalFloatRegs;

    /** Number of physical floating point registers. */
    int numPhysicalFloatRegs;

    /** Number of miscellaneous registers. */
    int numMiscRegs;

    /** Number of logical integer + float registers. */
    int numLogicalRegs;

    /** Number of physical integer + float registers. */
    int numPhysicalRegs;

    /** The logical index of the zero register. */
    int zeroRegIdx;
};

#endif
