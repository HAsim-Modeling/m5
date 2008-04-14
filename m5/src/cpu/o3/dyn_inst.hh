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


#ifndef __CPU_O3_DYN_INST_HH__
#define __CPU_O3_DYN_INST_HH__

#include "arch/isa_specific.hh"

#if THE_ISA == ALPHA_ISA
    template <class Impl> class AlphaDynInst;
    struct AlphaSimpleImpl;
    typedef AlphaDynInst<AlphaSimpleImpl> O3DynInst;
#elif THE_ISA == MIPS_ISA
    template <class Impl> class MipsDynInst;
    struct MipsSimpleImpl;
    typedef MipsDynInst<MipsSimpleImpl> O3DynInst;
#elif THE_ISA == SPARC_ISA
    template <class Impl> class SparcDynInst;
    struct SparcSimpleImpl;
    typedef SparcDynInst<SparcSimpleImpl> O3DynInst;
#elif THE_ISA == ARM_ISA
    template <class Impl> class ArmDynInst;
    struct ArmSimpleImpl;
    typedef ArmDynInst<ArmSimpleImpl> O3DynInst;
#else
    #error "O3DynInst not defined for this ISA"
#endif

#endif // __CPU_O3_DYN_INST_HH__
