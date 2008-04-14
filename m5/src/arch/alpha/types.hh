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
 * Authors: Nathan L. Binkert
 *          Steven K. Reinhardt
 */

#ifndef __ARCH_ALPHA_TYPES_HH__
#define __ARCH_ALPHA_TYPES_HH__

#include <inttypes.h>

namespace AlphaISA
{

    typedef uint32_t MachInst;
    typedef uint64_t ExtMachInst;
    typedef uint8_t  RegIndex;

    typedef uint64_t IntReg;
    typedef uint64_t LargestRead;

    // floating point register file entry type
    typedef double FloatReg;
    typedef uint64_t FloatRegBits;

    // control register file contents
    typedef uint64_t MiscReg;

    typedef union {
        IntReg  intreg;
        FloatReg   fpreg;
        MiscReg ctrlreg;
    } AnyReg;

    enum RegContextParam
    {
        CONTEXT_PALMODE
    };

    typedef bool RegContextVal;

    enum annotes {
        ANNOTE_NONE = 0,
        // An impossible number for instruction annotations
        ITOUCH_ANNOTE = 0xffffffff,
    };

    struct CoreSpecific {
	int core_type;
    };
} // namespace AlphaISA

#endif
