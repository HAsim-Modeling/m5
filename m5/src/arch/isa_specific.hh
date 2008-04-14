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

#ifndef __ARCH_ISA_SPECIFIC_HH__
#define __ARCH_ISA_SPECIFIC_HH__

//This file provides a mechanism for other source code to bring in
//files from the ISA being compiled in.

//These are constants so you can selectively compile code based on the isa.
//To use them, do something like:
//
//#if THE_ISA == YOUR_FAVORITE_ISA
//	conditional_code
//#endif
//
//Note that this is how this file sets up the TheISA macro.

//These macros have numerical values because otherwise the preprocessor
//would treat them as 0 in comparisons.
#define ALPHA_ISA 21064
#define SPARC_ISA 42
#define MIPS_ISA 34000
#define ARM_ISA 6

//These tell the preprocessor where to find the files of a particular
//ISA, and set the "TheISA" macro for use elsewhere.
#if THE_ISA == ALPHA_ISA
    #define TheISA AlphaISA
#elif THE_ISA == SPARC_ISA
    #define TheISA SparcISA
#elif THE_ISA == MIPS_ISA
    #define TheISA MipsISA
#elif THE_ISA == ARM_ISA
    #define TheISA ArmISA
#else
    #error "THE_ISA not set"
#endif

#endif
