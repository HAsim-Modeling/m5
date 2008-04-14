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

#include "arch/isa_specific.hh"
#include "cpu/o3/scoreboard.hh"

Scoreboard::Scoreboard(unsigned activeThreads,
                       unsigned _numLogicalIntRegs,
                       unsigned _numPhysicalIntRegs,
                       unsigned _numLogicalFloatRegs,
                       unsigned _numPhysicalFloatRegs,
                       unsigned _numMiscRegs,
                       unsigned _zeroRegIdx)
    : numLogicalIntRegs(_numLogicalIntRegs),
      numPhysicalIntRegs(_numPhysicalIntRegs),
      numLogicalFloatRegs(_numLogicalFloatRegs),
      numPhysicalFloatRegs(_numPhysicalFloatRegs),
      numMiscRegs(_numMiscRegs),
      zeroRegIdx(_zeroRegIdx)
{
    //Get Register Sizes
    numLogicalRegs = numLogicalIntRegs  + numLogicalFloatRegs;
    numPhysicalRegs = numPhysicalIntRegs  + numPhysicalFloatRegs;

    //Resize scoreboard appropriately
    regScoreBoard.resize(numPhysicalRegs + (numMiscRegs * activeThreads));

    //Initialize values
    for (int i=0; i < numLogicalIntRegs * activeThreads; i++) {
        regScoreBoard[i] = 1;
    }

    for (int i= numPhysicalIntRegs;
         i < numPhysicalIntRegs + (numLogicalFloatRegs * activeThreads);
         i++) {
        regScoreBoard[i] = 1;
    }

    for (int i = numPhysicalRegs;
         i < numPhysicalRegs + (numMiscRegs * activeThreads);
         i++) {
        regScoreBoard[i] = 1;
    }
}

std::string
Scoreboard::name() const
{
    return "cpu.scoreboard";
}

bool
Scoreboard::getReg(PhysRegIndex phys_reg)
{
#if THE_ISA == ALPHA_ISA
    // Always ready if int or fp zero reg.
    if (phys_reg == zeroRegIdx ||
        phys_reg == (zeroRegIdx + numPhysicalIntRegs)) {
        return 1;
    }
#else
    // Always ready if int zero reg.
    if (phys_reg == zeroRegIdx) {
        return 1;
    }
#endif

    return regScoreBoard[phys_reg];
}

void
Scoreboard::setReg(PhysRegIndex phys_reg)
{
    DPRINTF(Scoreboard, "Setting reg %i as ready\n", phys_reg);

    regScoreBoard[phys_reg] = 1;
}

void
Scoreboard::unsetReg(PhysRegIndex ready_reg)
{
#if THE_ISA == ALPHA_ISA
    if (ready_reg == zeroRegIdx ||
        ready_reg == (zeroRegIdx + numPhysicalIntRegs)) {
        // Don't do anything if int or fp zero reg.
        return;
    }
#else
    if (ready_reg == zeroRegIdx) {
        // Don't do anything if int zero reg.
        return;
    }
#endif

    regScoreBoard[ready_reg] = 0;
}
