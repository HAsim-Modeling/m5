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

#ifndef __ARCH_ALPHA_INTREGFILE_HH__
#define __ARCH_ALPHA_INTREGFILE_HH__

#include "arch/alpha/types.hh"

#include <iostream>
#include <cstring>

class Checkpoint;

namespace AlphaISA
{
    static inline std::string getIntRegName(RegIndex)
    {
        return "";
    }

    // redirected register map, really only used for the full system case.
    extern const int reg_redir[NumIntRegs];

    class IntRegFile
    {
      protected:
        IntReg regs[NumIntRegs];

      public:

        IntReg readReg(int intReg)
        {
            return regs[intReg];
        }

        void setReg(int intReg, const IntReg &val)
        {
            regs[intReg] = val;
        }

        void serialize(std::ostream &os);

        void unserialize(Checkpoint *cp, const std::string &section);

        void clear()
        { std::memset(regs, 0, sizeof(regs)); }
    };
}

#endif
