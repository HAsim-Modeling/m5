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
 */

#ifndef __ARCH_ALPHA_REGFILE_HH__
#define __ARCH_ALPHA_REGFILE_HH__

#include "arch/alpha/isa_traits.hh"
#include "arch/alpha/floatregfile.hh"
#include "arch/alpha/intregfile.hh"
#include "arch/alpha/miscregfile.hh"
#include "arch/alpha/types.hh"
#include "sim/faults.hh"

#include <string>

//XXX These should be implemented by someone who knows the alpha stuff better

class Checkpoint;
class ThreadContext;

namespace AlphaISA
{

    class RegFile {

      protected:
        Addr pc;			// program counter
        Addr npc;			// next-cycle program counter
        Addr nnpc;

      public:
        Addr readPC()
        {
            return pc;
        }

        void setPC(Addr val)
        {
            pc = val;
        }

        Addr readNextPC()
        {
            return npc;
        }

        void setNextPC(Addr val)
        {
            npc = val;
        }

        Addr readNextNPC()
        {
            return npc + sizeof(MachInst);
        }

        void setNextNPC(Addr val)
        { }

      protected:
        IntRegFile intRegFile;		// (signed) integer register file
        FloatRegFile floatRegFile;	// floating point register file
        MiscRegFile miscRegFile;	// control register file

      public:

#if FULL_SYSTEM
        int intrflag;			// interrupt flag
        inline int instAsid()
        { return miscRegFile.getInstAsid(); }
        inline int dataAsid()
        { return miscRegFile.getDataAsid(); }
#endif // FULL_SYSTEM

        void clear()
        {
            intRegFile.clear();
            floatRegFile.clear();
            miscRegFile.clear();
        }

        MiscReg readMiscRegNoEffect(int miscReg)
        {
            return miscRegFile.readRegNoEffect(miscReg);
        }

        MiscReg readMiscReg(int miscReg, ThreadContext *tc)
        {
            return miscRegFile.readReg(miscReg, tc);
        }

        void setMiscRegNoEffect(int miscReg, const MiscReg &val)
        {
            miscRegFile.setRegNoEffect(miscReg, val);
        }

        void setMiscReg(int miscReg, const MiscReg &val,
                ThreadContext * tc)
        {
            miscRegFile.setReg(miscReg, val, tc);
        }

        FloatReg readFloatReg(int floatReg)
        {
            return floatRegFile.d[floatReg];
        }

        FloatReg readFloatReg(int floatReg, int width)
        {
            return readFloatReg(floatReg);
        }

        FloatRegBits readFloatRegBits(int floatReg)
        {
            return floatRegFile.q[floatReg];
        }

        FloatRegBits readFloatRegBits(int floatReg, int width)
        {
            return readFloatRegBits(floatReg);
        }

        void setFloatReg(int floatReg, const FloatReg &val)
        {
            floatRegFile.d[floatReg] = val;
        }

        void setFloatReg(int floatReg, const FloatReg &val, int width)
        {
            setFloatReg(floatReg, val);
        }

        void setFloatRegBits(int floatReg, const FloatRegBits &val)
        {
            floatRegFile.q[floatReg] = val;
        }

        void setFloatRegBits(int floatReg, const FloatRegBits &val, int width)
        {
            setFloatRegBits(floatReg, val);
        }

        IntReg readIntReg(int intReg)
        {
            return intRegFile.readReg(intReg);
        }

        void setIntReg(int intReg, const IntReg &val)
        {
            intRegFile.setReg(intReg, val);
        }

        void serialize(std::ostream &os);
        void unserialize(Checkpoint *cp, const std::string &section);

        void changeContext(RegContextParam param, RegContextVal val)
        {
            //This would be an alternative place to call/implement
            //the swapPALShadow function
        }
    };

    static inline int flattenIntIndex(ThreadContext * tc, int reg)
    {
        return reg;
    }

    static inline int flattenFloatIndex(ThreadContext * tc, int reg)
    {
        return reg;
    }

    void copyRegs(ThreadContext *src, ThreadContext *dest);

    void copyMiscRegs(ThreadContext *src, ThreadContext *dest);
} // namespace AlphaISA

#endif
