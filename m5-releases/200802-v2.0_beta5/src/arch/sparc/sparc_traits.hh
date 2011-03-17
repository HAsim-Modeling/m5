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

#ifndef __ARCH_SPARC_SPARC_TRAITS_HH__
#define __ARCH_SPARC_SPARC_TRAITS_HH__

namespace SparcISA
{
    // Max trap levels
    const int MaxPTL = 2;
    const int MaxTL  = 6;
    const int MaxGL  = 3;
    const int MaxPGL = 2;

    // Number of register windows, can legally be 3 to 32
    const int NWindows = 8;
    //const int NumMicroIntRegs = 1;
    const int NumMicroIntRegs = 9;

//    const int NumRegularIntRegs = MaxGL * 8 + NWindows * 16;
//    const int NumMicroIntRegs = 1;
//    const int NumIntRegs =
//	NumRegularIntRegs +
//	NumMicroIntRegs;
//    const int NumFloatRegs = 64;
//    const int NumMiscRegs = 40;
}

#endif // __ARCH_SPARC_ISA_TRAITS_HH__
