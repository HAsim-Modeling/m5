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
 *          Kevin T. Lim
 */

#include "arch/alpha/isa_traits.hh"
#include "arch/alpha/intregfile.hh"
#include "sim/serialize.hh"

namespace AlphaISA
{
#if FULL_SYSTEM
    const int reg_redir[AlphaISA::NumIntRegs] = {
        /*  0 */ 0, 1, 2, 3, 4, 5, 6, 7,
        /*  8 */ 32, 33, 34, 35, 36, 37, 38, 15,
        /* 16 */ 16, 17, 18, 19, 20, 21, 22, 23,
        /* 24 */ 24, 39, 26, 27, 28, 29, 30, 31 };
#else
    const int reg_redir[AlphaISA::NumIntRegs] = {
        /*  0 */ 0, 1, 2, 3, 4, 5, 6, 7,
        /*  8 */ 8, 9, 10, 11, 12, 13, 14, 15,
        /* 16 */ 16, 17, 18, 19, 20, 21, 22, 23,
        /* 24 */ 24, 25, 26, 27, 28, 29, 30, 31 };
#endif

    void
    IntRegFile::serialize(std::ostream &os)
    {
        SERIALIZE_ARRAY(regs, NumIntRegs);
    }

    void
    IntRegFile::unserialize(Checkpoint *cp, const std::string &section)
    {
        UNSERIALIZE_ARRAY(regs, NumIntRegs);
    }
}

