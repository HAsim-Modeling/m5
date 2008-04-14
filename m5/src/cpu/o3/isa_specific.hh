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
#include "cpu/base.hh"

#if THE_ISA == ALPHA_ISA
    #include "cpu/o3/alpha/cpu.hh"
    #include "cpu/o3/alpha/impl.hh"
    #include "cpu/o3/alpha/params.hh"
    #include "cpu/o3/alpha/dyn_inst.hh"
#elif THE_ISA == MIPS_ISA
    #include "cpu/o3/mips/cpu.hh"
    #include "cpu/o3/mips/impl.hh"
    #include "cpu/o3/mips/params.hh"
    #include "cpu/o3/mips/dyn_inst.hh"
#elif THE_ISA == SPARC_ISA
    #include "cpu/o3/sparc/cpu.hh"
    #include "cpu/o3/sparc/impl.hh"
    #include "cpu/o3/sparc/params.hh"
    #include "cpu/o3/sparc/dyn_inst.hh"
#else
    #error "ISA-specific header files O3CPU not defined ISA"
#endif
