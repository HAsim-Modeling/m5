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
 *          Korey L. Sewell
 */

#include "arch/mips/regfile/float_regfile.hh"
#include "sim/serialize.hh"

using namespace MipsISA;
using namespace std;

void
FloatRegFile::clear()
{
    bzero(&regs, sizeof(regs));
}

double
FloatRegFile::readReg(int floatReg, int width, unsigned tid)
{
    switch(width)
    {
      case SingleWidth:
        {
            void *float_ptr = &regs[floatReg];
            return *(float *) float_ptr;
        }

      case DoubleWidth:
        {
            uint64_t double_val = (FloatReg64)regs[floatReg + 1] << 32 | regs[floatReg];
            void *double_ptr = &double_val;
            return *(double *) double_ptr;
        }

      default:
        panic("Attempted to read a %d bit floating point register!", width);
    }
}

FloatRegBits
FloatRegFile::readRegBits(int floatReg, int width, unsigned tid)
{
    if (floatReg < NumFloatArchRegs - 1) {
        switch(width)
        {
          case SingleWidth:
            return regs[floatReg];

          case DoubleWidth:
            return (FloatReg64)regs[floatReg + 1] << 32 | regs[floatReg];

          default:
            panic("Attempted to read a %d bit floating point register!", width);
        }
    } else {
        if (width > SingleWidth)
            assert("Control Regs are only 32 bits wide");

        return regs[floatReg];
    }
}

Fault
FloatRegFile::setReg(int floatReg, const FloatRegVal &val, int width, unsigned tid)
{
    using namespace std;
    switch(width)
    {
      case SingleWidth:
        {
            float temp = val;
            void *float_ptr = &temp;
            regs[floatReg] = *(FloatReg32 *) float_ptr;
            break;
        }

      case DoubleWidth:
        {
            const void *double_ptr = &val;
            FloatReg64 temp_double = *(FloatReg64 *) double_ptr;
            regs[floatReg + 1] = bits(temp_double, 63, 32);
            regs[floatReg] = bits(temp_double, 31, 0);
            break;
        }

      default:
        panic("Attempted to read a %d bit floating point register!", width);
    }

    return NoFault;
}

Fault
FloatRegFile::setRegBits(int floatReg, const FloatRegBits &val, int width, unsigned tid)
{
    using namespace std;

    switch(width)
    {
      case SingleWidth:
        regs[floatReg] = val;
        break;

      case DoubleWidth:
        regs[floatReg + 1] = bits(val, 63, 32);
        regs[floatReg] = bits(val, 31, 0);
        break;

      default:
        panic("Attempted to read a %d bit floating point register!", width);
    }
    return NoFault;
}

void
FloatRegFile::serialize(std::ostream &os)
{
    SERIALIZE_ARRAY(regs, NumFloatRegs);
}

void
FloatRegFile::unserialize(Checkpoint *cp, const std::string &section)
{
    UNSERIALIZE_ARRAY(regs, NumFloatRegs);
}
