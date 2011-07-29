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

#include "arch/sparc/floatregfile.hh"
#include "base/trace.hh"
#include "sim/byteswap.hh"
#include "sim/serialize.hh"

#include <string.h>

using namespace SparcISA;
using namespace std;

class Checkpoint;

string SparcISA::getFloatRegName(RegIndex index)
{
    static std::string floatRegName[NumFloatRegs] =
        {"f0", "f1", "f2", "f3", "f4", "f5", "f6", "f7",
         "f8", "f9", "f10", "f11", "f12", "f13", "f14", "f15",
         "f16", "f17", "f18", "f19", "f20", "f21", "f22", "f23",
         "f24", "f25", "f26", "f27", "f28", "f29", "f30", "f31",
         "f32", "f33", "f34", "f35", "f36", "f37", "f38", "f39",
         "f40", "f41", "f42", "f43", "f44", "f45", "f46", "f47",
         "f48", "f49", "f50", "f51", "f52", "f53", "f54", "f55",
         "f56", "f57", "f58", "f59", "f60", "f61", "f62", "f63"};
    return floatRegName[index];
}

void FloatRegFile::clear()
{
    memset(regSpace, 0, sizeof(regSpace));
}

FloatReg FloatRegFile::readReg(int floatReg, int width)
{
    //In each of these cases, we have to copy the value into a temporary
    //variable. This is because we may otherwise try to access an
    //unaligned portion of memory.
    FloatReg result;
    switch(width)
    {
      case SingleWidth:
        uint32_t result32;
        float32_t fresult32;
        memcpy(&result32, regSpace + 4 * floatReg, sizeof(result32));
        result32 = htog(result32);
        memcpy(&fresult32, &result32, sizeof(result32));
        result = fresult32;
        DPRINTF(Sparc, "Read FP32 register %d = [%f]0x%x\n", floatReg, result, result32);
        break;
      case DoubleWidth:
        uint64_t result64;
        float64_t fresult64;
        memcpy(&result64, regSpace + 4 * floatReg, sizeof(result64));
        result64 = htog(result64);
        memcpy(&fresult64, &result64, sizeof(result64));
        result = fresult64;
        DPRINTF(Sparc, "Read FP64 register %d = [%f]0x%x\n", floatReg, result, result64);
        break;
      case QuadWidth:
        panic("Quad width FP not implemented.");
        break;
      default:
        panic("Attempted to read a %d bit floating point register!", width);
    }
    return result;
}

FloatRegBits FloatRegFile::readRegBits(int floatReg, int width)
{
    //In each of these cases, we have to copy the value into a temporary
    //variable. This is because we may otherwise try to access an
    //unaligned portion of memory.
    FloatRegBits result;
    switch(width)
    {
      case SingleWidth:
        uint32_t result32;
        memcpy(&result32, regSpace + 4 * floatReg, sizeof(result32));
        result = htog(result32);
        DPRINTF(Sparc, "Read FP32 bits register %d = 0x%x\n", floatReg, result);
        break;
      case DoubleWidth:
        uint64_t result64;
        memcpy(&result64, regSpace + 4 * floatReg, sizeof(result64));
        result = htog(result64);
        DPRINTF(Sparc, "Read FP64 bits register %d = 0x%x\n", floatReg, result);
        break;
      case QuadWidth:
        panic("Quad width FP not implemented.");
        break;
      default:
        panic("Attempted to read a %d bit floating point register!", width);
    }
    return result;
}

Fault FloatRegFile::setReg(int floatReg, const FloatReg &val, int width)
{
    //In each of these cases, we have to copy the value into a temporary
    //variable. This is because we may otherwise try to access an
    //unaligned portion of memory.

    uint32_t result32;
    uint64_t result64;
    float32_t fresult32;
    float64_t fresult64;
    switch(width)
    {
      case SingleWidth:
        fresult32 = val;
        memcpy(&result32, &fresult32, sizeof(result32));
        result32 = gtoh(result32);
        memcpy(regSpace + 4 * floatReg, &result32, sizeof(result32));
        DPRINTF(Sparc, "Write FP64 register %d = 0x%x\n", floatReg, result32);
        break;
      case DoubleWidth:
        fresult64 = val;
        memcpy(&result64, &fresult64, sizeof(result64));
        result64 = gtoh(result64);
        memcpy(regSpace + 4 * floatReg, &result64, sizeof(result64));
        DPRINTF(Sparc, "Write FP64 register %d = 0x%x\n", floatReg, result64);
        break;
      case QuadWidth:
        panic("Quad width FP not implemented.");
        break;
      default:
        panic("Attempted to read a %d bit floating point register!", width);
    }
    return NoFault;
}

Fault FloatRegFile::setRegBits(int floatReg, const FloatRegBits &val, int width)
{
    //In each of these cases, we have to copy the value into a temporary
    //variable. This is because we may otherwise try to access an
    //unaligned portion of memory.
    uint32_t result32;
    uint64_t result64;
    switch(width)
    {
      case SingleWidth:
        result32 = gtoh((uint32_t)val);
        memcpy(regSpace + 4 * floatReg, &result32, sizeof(result32));
        DPRINTF(Sparc, "Write FP64 bits register %d = 0x%x\n", floatReg, result32);
        break;
      case DoubleWidth:
        result64 = gtoh((uint64_t)val);
        memcpy(regSpace + 4 * floatReg, &result64, sizeof(result64));
        DPRINTF(Sparc, "Write FP64 bits register %d = 0x%x\n", floatReg, result64);
        break;
      case QuadWidth:
        panic("Quad width FP not implemented.");
        break;
      default:
        panic("Attempted to read a %d bit floating point register!", width);
    }
    return NoFault;
}

void FloatRegFile::serialize(std::ostream &os)
{
    uint8_t *float_reg = (uint8_t*)regSpace;
    SERIALIZE_ARRAY(float_reg,
            SingleWidth / 8 * NumFloatRegs);
}

void FloatRegFile::unserialize(Checkpoint *cp, const std::string &section)
{
    uint8_t *float_reg = (uint8_t*)regSpace;
    UNSERIALIZE_ARRAY(float_reg,
            SingleWidth / 8 * NumFloatRegs);
}
