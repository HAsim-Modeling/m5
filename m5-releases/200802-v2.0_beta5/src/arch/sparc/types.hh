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

#ifndef __ARCH_SPARC_TYPES_HH__
#define __ARCH_SPARC_TYPES_HH__

#include <inttypes.h>
#include "base/bigint.hh"

namespace SparcISA
{
    typedef uint32_t MachInst;
    typedef uint64_t ExtMachInst;

    typedef uint64_t IntReg;
    typedef Twin64_t LargestRead;
    typedef uint64_t MiscReg;
    typedef double FloatReg;
    typedef uint64_t FloatRegBits;
    typedef union
    {
        IntReg intReg;
        FloatReg fpreg;
        MiscReg ctrlreg;
    } AnyReg;

    enum RegContextParam
    {
        CONTEXT_CWP,
        CONTEXT_GLOBALS
    };

    typedef int RegContextVal;

    typedef uint16_t RegIndex;

    struct CoreSpecific {
	int core_type;
    };
}

#endif