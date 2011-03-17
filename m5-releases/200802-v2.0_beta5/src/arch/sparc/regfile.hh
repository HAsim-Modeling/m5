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

#ifndef __ARCH_SPARC_REGFILE_HH__
#define __ARCH_SPARC_REGFILE_HH__

#include "arch/sparc/floatregfile.hh"
#include "arch/sparc/intregfile.hh"
#include "arch/sparc/isa_traits.hh"
#include "arch/sparc/miscregfile.hh"
#include "arch/sparc/types.hh"
#include "sim/host.hh"

#include <string>

class Checkpoint;

namespace SparcISA
{
    class RegFile
    {
      protected:
        Addr pc;		// Program Counter
        Addr npc;		// Next Program Counter
        Addr nnpc;

      public:
        Addr readPC();
        void setPC(Addr val);

        Addr readNextPC();
        void setNextPC(Addr val);

        Addr readNextNPC();
        void setNextNPC(Addr val);

      protected:
        IntRegFile intRegFile;		// integer register file
        FloatRegFile floatRegFile;	// floating point register file
        MiscRegFile miscRegFile;	// control register file

      public:

        void clear();

        int FlattenIntIndex(int reg);

        MiscReg readMiscRegNoEffect(int miscReg);

        MiscReg readMiscReg(int miscReg, ThreadContext *tc);

        void setMiscRegNoEffect(int miscReg, const MiscReg &val);

        void setMiscReg(int miscReg, const MiscReg &val,
                ThreadContext * tc);

        int instAsid()
        {
            return miscRegFile.getInstAsid();
        }

        int dataAsid()
        {
            return miscRegFile.getDataAsid();
        }

        FloatReg readFloatReg(int floatReg, int width);

        FloatReg readFloatReg(int floatReg);

        FloatRegBits readFloatRegBits(int floatReg, int width);

        FloatRegBits readFloatRegBits(int floatReg);

        void setFloatReg(int floatReg, const FloatReg &val, int width);

        void setFloatReg(int floatReg, const FloatReg &val);

        void setFloatRegBits(int floatReg, const FloatRegBits &val, int width);

        void setFloatRegBits(int floatReg, const FloatRegBits &val);

        IntReg readIntReg(int intReg);

        void setIntReg(int intReg, const IntReg &val);

        void serialize(std::ostream &os);
        void unserialize(Checkpoint *cp, const std::string &section);

      public:

        void changeContext(RegContextParam param, RegContextVal val);
    };

    int flattenIntIndex(ThreadContext * tc, int reg);

    static inline int
    flattenFloatIndex(ThreadContext * tc, int reg)
    {
        return reg;
    }

    void copyRegs(ThreadContext *src, ThreadContext *dest);

    void copyMiscRegs(ThreadContext *src, ThreadContext *dest);

} // namespace SparcISA

#endif
