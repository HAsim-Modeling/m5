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
 * Authors: Steven K. Reinhardt
 *          Gabe M. Black
 */

#ifndef __ARCH_ALPHA_MISCREGFILE_HH__
#define __ARCH_ALPHA_MISCREGFILE_HH__

#include "arch/alpha/ipr.hh"
#include "arch/alpha/types.hh"
#include "sim/host.hh"
#include "sim/serialize.hh"

#include <iostream>

class Checkpoint;
class ThreadContext;

namespace AlphaISA
{
    enum MiscRegIndex
    {
        MISCREG_FPCR = NumInternalProcRegs,
        MISCREG_UNIQ,
        MISCREG_LOCKFLAG,
        MISCREG_LOCKADDR,
        MISCREG_INTR
    };

    static inline std::string getMiscRegName(RegIndex)
    {
        return "";
    }

    class MiscRegFile {
      protected:
        uint64_t	fpcr;		// floating point condition codes
        uint64_t	uniq;		// process-unique register
        bool		lock_flag;	// lock flag for LL/SC
        Addr		lock_addr;	// lock address for LL/SC
        int		intr_flag;

      public:
        MiscRegFile()
        {
            initializeIprTable();
        }

        MiscReg readRegNoEffect(int misc_reg);

        MiscReg readReg(int misc_reg, ThreadContext *tc);

        //These functions should be removed once the simplescalar cpu model
        //has been replaced.
        int getInstAsid();
        int getDataAsid();

        void setRegNoEffect(int misc_reg, const MiscReg &val);

        void setReg(int misc_reg, const MiscReg &val,
                ThreadContext *tc);

        void clear()
        {
            fpcr = uniq = 0;
            lock_flag = 0;
            lock_addr = 0;
            intr_flag = 0;
        }

        void serialize(std::ostream &os);

        void unserialize(Checkpoint *cp, const std::string &section);
      protected:
        typedef uint64_t InternalProcReg;

        InternalProcReg ipr[NumInternalProcRegs]; // Internal processor regs

      private:
        InternalProcReg readIpr(int idx, ThreadContext *tc);

        void setIpr(int idx, InternalProcReg val, ThreadContext *tc);
        friend class RegFile;
    };

    void copyIprs(ThreadContext *src, ThreadContext *dest);

}

#endif
