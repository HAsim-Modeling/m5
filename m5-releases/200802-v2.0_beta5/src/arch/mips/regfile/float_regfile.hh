/*
 * Copyright (c) 2006
 * The Regents of The University of Michigan
 * All Rights Reserved
 *
 * This code is part of the M5 simulator, developed by Nathan Binkert,
 * Erik Hallnor, Steve Raasch, and Steve Reinhardt, with contributions
 * from Ron Dreslinski, Dave Greene, Lisa Hsu, Kevin Lim, Ali Saidi,
 * and Andrew Schultz.
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
 */

/*
 * Copyright (c) 2007 MIPS Technologies, Inc.  All Rights Reserved
 *
 * This software is part of the M5 simulator.
 *
 * THIS IS A LEGAL AGREEMENT.  BY DOWNLOADING, USING, COPYING, CREATING
 * DERIVATIVE WORKS, AND/OR DISTRIBUTING THIS SOFTWARE YOU ARE AGREEING
 * TO THESE TERMS AND CONDITIONS.
 *
 * Permission is granted to use, copy, create derivative works and
 * distribute this software and such derivative works for any purpose,
 * so long as (1) the copyright notice above, this grant of permission,
 * and the disclaimer below appear in all copies and derivative works
 * made, (2) the copyright notice above is augmented as appropriate to
 * reflect the addition of any new copyrightable work in a derivative
 * work (e.g., Copyright (c) <Publication Year> Copyright Owner), and (3)
 * the name of MIPS Technologies, Inc. (¡ÈMIPS¡É) is not used in any
 * advertising or publicity pertaining to the use or distribution of
 * this software without specific, written prior authorization.
 *
 * THIS SOFTWARE IS PROVIDED ¡ÈAS IS.¡É  MIPS MAKES NO WARRANTIES AND
 * DISCLAIMS ALL WARRANTIES, WHETHER EXPRESS, STATUTORY, IMPLIED OR
 * OTHERWISE, INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND
 * NON-INFRINGEMENT OF THIRD PARTY RIGHTS, REGARDING THIS SOFTWARE.
 * IN NO EVENT SHALL MIPS BE LIABLE FOR ANY DAMAGES, INCLUDING DIRECT,
 * INDIRECT, INCIDENTAL, CONSEQUENTIAL, SPECIAL, OR PUNITIVE DAMAGES OF
 * ANY KIND OR NATURE, ARISING OUT OF OR IN CONNECTION WITH THIS AGREEMENT,
 * THIS SOFTWARE AND/OR THE USE OF THIS SOFTWARE, WHETHER SUCH LIABILITY
 * IS ASSERTED ON THE BASIS OF CONTRACT, TORT (INCLUDING NEGLIGENCE OR
 * STRICT LIABILITY), OR OTHERWISE, EVEN IF MIPS HAS BEEN WARNED OF THE
 * POSSIBILITY OF ANY SUCH LOSS OR DAMAGE IN ADVANCE.
 *
 * Authors: Korey Sewell
 *
 */

#ifndef __ARCH_MIPS_REGFILE_FLOAT_REGFILE_HH__
#define __ARCH_MIPS_REGFILE_FLOAT_REGFILE_HH__

#include "arch/mips/types.hh"
#include "arch/mips/isa_traits.hh"
#include "base/misc.hh"
#include "base/bitfield.hh"
#include "sim/faults.hh"

#include <string>

class Checkpoint;

namespace MipsISA
{
    static inline std::string getFloatRegName(RegIndex)
    {
	return "";
    }

    const uint32_t MIPS32_QNAN = 0x7fbfffff;
    const uint64_t MIPS64_QNAN = ULL(0x7fbfffffffffffff);

    enum FPControlRegNums {
       FIR = NumFloatArchRegs,
       FCCR,
       FEXR,
       FENR,
       FCSR
    };

    enum FCSRBits {
	Inexact = 1,
	Underflow,
	Overflow,
	DivideByZero,
	Invalid,
	Unimplemented
    };

    enum FCSRFields {
	Flag_Field = 1,
	Enable_Field = 6,
	Cause_Field = 11
    };

    const int SingleWidth = 32;
    const int SingleBytes = SingleWidth / 4;

    const int DoubleWidth = 64;
    const int DoubleBytes = DoubleWidth / 4;

    const int QuadWidth = 128;
    const int QuadBytes = QuadWidth / 4;

    class FloatRegFile
    {
      protected:
	FloatReg32 regs[NumFloatRegs];

      public:

        void clear();
        double readReg(int floatReg, int width, unsigned tid = 0);
        FloatRegBits readRegBits(int floatReg, int width, unsigned tid = 0);
        Fault setReg(int floatReg, const FloatRegVal &val, int width, unsigned tid = 0);
        Fault setRegBits(int floatReg, const FloatRegBits &val, int width, unsigned tid = 0);

	void serialize(std::ostream &os);
	void unserialize(Checkpoint *cp, const std::string &section);
    };

} // namespace MipsISA

#endif
