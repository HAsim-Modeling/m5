/*
 * Copyright (c) 2003, 2004, 2005
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
 *          Ali G. Saidi
 */

#include "arch/sparc/intregfile.hh"
#include "base/trace.hh"
#include "base/misc.hh"
#include "sim/serialize.hh"

#include <string.h>

using namespace SparcISA;
using namespace std;

class Checkpoint;

string SparcISA::getIntRegName(RegIndex index)
{
    static std::string intRegName[NumIntArchRegs] =
        {"g0", "g1", "g2", "g3", "g4", "g5", "g6", "g7",
         "o0", "o1", "o2", "o3", "o4", "o5", "o6", "o7",
         "l0", "l1", "l2", "l3", "l4", "l5", "l6", "l7",
         "i0", "i1", "i2", "i3", "i4", "i5", "i6", "i7"};
    return intRegName[index];
}

int IntRegFile::flattenIndex(int reg)
{
    int flatIndex = offset[reg >> FrameOffsetBits]
        | (reg & FrameOffsetMask);
    DPRINTF(Sparc, "Flattened index %d into %d.\n", reg, flatIndex);
    return flatIndex;
}

void IntRegFile::clear()
{
    int x;
    for (x = 0; x < MaxGL; x++)
        memset(regGlobals[x], 0, sizeof(IntReg) * RegsPerFrame);
    for(int x = 0; x < 2 * NWindows; x++)
        memset(regSegments[x], 0, sizeof(IntReg) * RegsPerFrame);
    memset(regs, 0, sizeof(IntReg) * NumIntRegs);
}

IntRegFile::IntRegFile()
{
    offset[Globals] = 0;
    regView[Globals] = regGlobals[0];
    setCWP(0);
    clear();
}

IntReg IntRegFile::readReg(int intReg)
{
    DPRINTF(Sparc, "Read register %d = 0x%x\n", intReg, regs[intReg]);
    return regs[intReg];
    /* XXX Currently not used. When used again regView/offset need to be
     * serialized!
    IntReg val;
    if(intReg < NumIntArchRegs)
        val = regView[intReg >> FrameOffsetBits][intReg & FrameOffsetMask];
    else if((intReg -= NumIntArchRegs) < NumMicroIntRegs)
        val = microRegs[intReg];
    else
        panic("Tried to read non-existant integer register %d, %d\n",
                NumIntArchRegs + NumMicroIntRegs + intReg, intReg);

    DPRINTF(Sparc, "Read register %d = 0x%x\n", intReg, val);
    return val;
    */
}

void IntRegFile::setReg(int intReg, const IntReg &val)
{
    if(intReg)
    {
        DPRINTF(Sparc, "Wrote register %d = 0x%x\n", intReg, val);
        regs[intReg] = val;
    }
    return;
    /* XXX Currently not used. When used again regView/offset need to be
     * serialized!
    if(intReg)
    {
        DPRINTF(Sparc, "Wrote register %d = 0x%x\n", intReg, val);
        if(intReg < NumIntArchRegs)
            regView[intReg >> FrameOffsetBits][intReg & FrameOffsetMask] = val;
        else if((intReg -= NumIntArchRegs) < NumMicroIntRegs)
            microRegs[intReg] = val;
        else
            panic("Tried to set non-existant integer register\n");
    } */
}

//This doesn't effect the actual CWP register.
//It's purpose is to adjust the view of the register file
//to what it would be if CWP = cwp.
void IntRegFile::setCWP(int cwp)
{
    int index = ((NWindows - cwp) % NWindows) * 2;
    if (index < 0)
        panic("Index less than 0. cwp=%d nwin=%d\n", cwp, NWindows);
    offset[Outputs] = FrameOffset + (index * RegsPerFrame);
    offset[Locals] = FrameOffset + ((index+1) * RegsPerFrame);
    offset[Inputs] = FrameOffset +
        (((index+2) % (NWindows * 2)) * RegsPerFrame);
    regView[Outputs] = regSegments[index];
    regView[Locals] = regSegments[index+1];
    regView[Inputs] = regSegments[(index+2) % (NWindows * 2)];

    DPRINTF(Sparc, "Changed the CWP value to %d\n", cwp);
}

void IntRegFile::setGlobals(int gl)
{
    DPRINTF(Sparc, "Now using %d globals\n", gl);

    regView[Globals] = regGlobals[gl];
    offset[Globals] = RegGlobalOffset + gl * RegsPerFrame;

    if (regView[Globals] == regView[Inputs] ||
        regView[Globals] == regView[Locals] ||
        regView[Globals] == regView[Outputs] )
        panic("Two register arrays set to the same thing!\n");
}

void IntRegFile::serialize(std::ostream &os)
{
    SERIALIZE_ARRAY(regs, NumIntRegs);
    SERIALIZE_ARRAY(microRegs, NumMicroIntRegs);

    /* the below doesn't seem needed unless gabe makes regview work*/
    unsigned int x;
    for(x = 0; x < MaxGL; x++)
        SERIALIZE_ARRAY(regGlobals[x], RegsPerFrame);
    for(x = 0; x < 2 * NWindows; x++)
        SERIALIZE_ARRAY(regSegments[x], RegsPerFrame);
}

void IntRegFile::unserialize(Checkpoint *cp, const std::string &section)
{
    UNSERIALIZE_ARRAY(regs, NumIntRegs);
    UNSERIALIZE_ARRAY(microRegs, NumMicroIntRegs);

    /* the below doesn't seem needed unless gabe makes regview work*/
    unsigned int x;
    for(x = 0; x < MaxGL; x++)
        UNSERIALIZE_ARRAY(regGlobals[x], RegsPerFrame);
    for(unsigned int x = 0; x < 2 * NWindows; x++)
        UNSERIALIZE_ARRAY(regSegments[x], RegsPerFrame);
}
