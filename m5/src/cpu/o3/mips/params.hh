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
 * Authors: Kevin T. Lim
 *          Korey L. Sewell
 */

#ifndef __CPU_O3_MIPS_PARAMS_HH__
#define __CPU_O3_MIPS_PARAMS_HH__

#include "cpu/o3/cpu.hh"
#include "cpu/o3/params.hh"

//Forward declarations
namespace MipsISA
{
    class MipsDTB;
    class MipsITB;
}
class MemObject;
class Process;
class System;

/**
 * This file defines the parameters that will be used for the MipsO3CPU.
 * This must be defined externally so that the Impl can have a params class
 * defined that it can pass to all of the individual stages.
 */

class MipsSimpleParams : public O3Params
{
  public:
    MipsSimpleParams() {}

    //Full System Paramater Objects place here
    MipsISA::ITB *itb;
    MipsISA::DTB *dtb;
};

#endif // __CPU_O3_MIPS_PARAMS_HH__
