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

#ifndef __ARCH_SPARC_FLOATREGFILE_HH__
#define __ARCH_SPARC_FLOATREGFILE_HH__

#include "arch/sparc/faults.hh"
#include "arch/sparc/isa_traits.hh"
#include "arch/sparc/types.hh"

#include <string>

class Checkpoint;

namespace SparcISA
{
    std::string getFloatRegName(RegIndex);

    const int NumFloatArchRegs = 64;
    const int NumFloatRegs = 64;

    typedef float float32_t;
    typedef double float64_t;
    //FIXME long double refers to a 10 byte float, rather than a
    //16 byte float as required. This data type may have to be emulated.
    typedef double float128_t;

    class FloatRegFile
    {
      public:
        static const int SingleWidth = 32;
        static const int DoubleWidth = 64;
        static const int QuadWidth = 128;

      protected:

        //Since the floating point registers overlap each other,
        //A generic storage space is used. The float to be returned is
        //pulled from the appropriate section of this region.
        char regSpace[(SingleWidth / 8) * NumFloatRegs];

      public:

        void clear();

        FloatReg readReg(int floatReg, int width);

        FloatRegBits readRegBits(int floatReg, int width);

        Fault setReg(int floatReg, const FloatReg &val, int width);

        Fault setRegBits(int floatReg, const FloatRegBits &val, int width);

        void serialize(std::ostream &os);

        void unserialize(Checkpoint *cp, const std::string &section);
    };
}

#endif
