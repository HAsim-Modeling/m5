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

#ifndef __ARCH_ALPHA_FLOATREGFILE_HH__
#define __ARCH_ALPHA_FLOATREGFILE_HH__

#include "arch/alpha/isa_traits.hh"
#include "arch/alpha/types.hh"

#include <cstring>
#include <iostream>

class Checkpoint;

namespace AlphaISA
{
    static inline std::string getFloatRegName(RegIndex)
    {
        return "";
    }

    class FloatRegFile
    {
      public:

        union {
            uint64_t q[NumFloatRegs];	// integer qword view
            double d[NumFloatRegs];	// double-precision floating point view
        };

        void serialize(std::ostream &os);

        void unserialize(Checkpoint *cp, const std::string &section);

        void clear()
        { std::memset(d, 0, sizeof(d)); }
    };
}

#endif
