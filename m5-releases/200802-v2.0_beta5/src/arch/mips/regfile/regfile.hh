/*
 * Copyright (c) 2006
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


#ifndef __ARCH_MIPS_REGFILE_REGFILE_HH__
#define __ARCH_MIPS_REGFILE_REGFILE_HH__

#include "arch/mips/types.hh"
#include "arch/mips/isa_traits.hh"
//#include "arch/mips/mt.hh"
#include "arch/mips/regfile/int_regfile.hh"
#include "arch/mips/regfile/float_regfile.hh"
#include "arch/mips/regfile/misc_regfile.hh"
//#include "cpu/base.hh"
#include "sim/faults.hh"

class Checkpoint;
class BaseCPU;

namespace MipsISA
{
    class RegFile {
      protected:
        Addr pc;			// program counter
        Addr npc;			// next-cycle program counter
        Addr nnpc;			// next-next-cycle program counter
                                        // used to implement branch delay slot
                                        // not real register

        IntRegFile intRegFile;		// (signed) integer register file
        FloatRegFile floatRegFile;	// floating point register file
        MiscRegFile miscRegFile;	// control register file

      public:
        void clear();
        void reset(std::string core_name, unsigned num_threads, unsigned num_vpes, BaseCPU *_cpu);
        MiscRegFile *getMiscRegFilePtr();

        IntReg readIntReg(int intReg);
        Fault setIntReg(int intReg, const IntReg &val);


        MiscReg readMiscRegNoEffect(int miscReg, unsigned tid = 0);
        MiscReg readMiscReg(int miscReg, ThreadContext *tc,
                            unsigned tid = 0);
        void setMiscRegNoEffect(int miscReg, const MiscReg &val, unsigned tid = 0);
        void setMiscReg(int miscReg, const MiscReg &val,
                        ThreadContext * tc, unsigned tid = 0);

        FloatRegVal readFloatReg(int floatReg);
        FloatRegVal readFloatReg(int floatReg, int width);
        FloatRegBits readFloatRegBits(int floatReg);
        FloatRegBits readFloatRegBits(int floatReg, int width);
        Fault setFloatReg(int floatReg, const FloatRegVal &val);
        Fault setFloatReg(int floatReg, const FloatRegVal &val, int width);
        Fault setFloatRegBits(int floatReg, const FloatRegBits &val);
        Fault setFloatRegBits(int floatReg, const FloatRegBits &val, int width);


        void setShadowSet(int css);

        int instAsid();
        int dataAsid();

      public:
        Addr readPC();
        void setPC(Addr val);

        Addr readNextPC();
        void setNextPC(Addr val);

        Addr readNextNPC();
        void setNextNPC(Addr val);

        void serialize(std::ostream &os);
        void unserialize(Checkpoint *cp, const std::string &section);

        void changeContext(RegContextParam param, RegContextVal val)
        {
        }

    };

} // namespace MipsISA

#endif
