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

#ifndef __ARCH_SPARC_INTREGFILE_HH__
#define __ARCH_SPARC_INTREGFILE_HH__

#include "arch/sparc/isa_traits.hh"
#include "arch/sparc/types.hh"
#include "base/bitfield.hh"

#include <string>

class Checkpoint;

namespace SparcISA
{
    class RegFile;

    //This function translates integer register file indices into names
    std::string getIntRegName(RegIndex);

    const int NumIntArchRegs = 32;
    const int NumIntRegs = (MaxGL + 1) * 8 + NWindows * 16 + NumMicroIntRegs;

    class IntRegFile
    {
      private:
        friend class RegFile;
      protected:
        //The number of bits needed to index into each 8 register frame
        static const int FrameOffsetBits = 3;
        //The number of bits to choose between the 4 sets of 8 registers
        static const int FrameNumBits = 2;

        //The number of registers per "frame" (8)
        static const int RegsPerFrame = 1 << FrameOffsetBits;
        //A mask to get the frame number
        static const uint64_t FrameNumMask =
                (FrameNumBits == sizeof(int)) ?
                (unsigned int)(-1) :
                (1 << FrameNumBits) - 1;
        static const uint64_t FrameOffsetMask =
                (FrameOffsetBits == sizeof(int)) ?
                (unsigned int)(-1) :
                (1 << FrameOffsetBits) - 1;

        IntReg regGlobals[MaxGL+1][RegsPerFrame];
        IntReg regSegments[2 * NWindows][RegsPerFrame];
        IntReg microRegs[NumMicroIntRegs];
        IntReg regs[NumIntRegs];

        enum regFrame {Globals, Outputs, Locals, Inputs, NumFrames};

        IntReg * regView[NumFrames];

        static const int RegGlobalOffset = 0;
        static const int FrameOffset = (MaxGL + 1) * RegsPerFrame;
        int offset[NumFrames];

      public:

        int flattenIndex(int reg);

        void clear();

        IntRegFile();

        IntReg readReg(int intReg);

        void setReg(int intReg, const IntReg &val);

        void serialize(std::ostream &os);

        void unserialize(Checkpoint *cp, const std::string &section);

      protected:
        //This doesn't effect the actual CWP register.
        //It's purpose is to adjust the view of the register file
        //to what it would be if CWP = cwp.
        void setCWP(int cwp);

        void setGlobals(int gl);
    };
}

#endif
