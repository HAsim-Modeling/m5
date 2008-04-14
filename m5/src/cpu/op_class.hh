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
 */

#ifndef __CPU__OP_CLASS_HH__
#define __CPU__OP_CLASS_HH__

#include "enums/OpClass.hh"

/*
 * Do a bunch of wonky stuff to maintain backward compatability so I
 * don't have to change code in a zillion places.
 */
using Enums::OpClass;
using Enums::No_OpClass;
using Enums::Num_OpClass;

const OpClass IntAluOp = Enums::IntAlu;
const OpClass IntMultOp = Enums::IntMult;
const OpClass IntDivOp = Enums::IntDiv;
const OpClass FloatAddOp = Enums::FloatAdd;
const OpClass FloatCmpOp = Enums::FloatCmp;
const OpClass FloatCvtOp = Enums::FloatCvt;
const OpClass FloatMultOp = Enums::FloatMult;
const OpClass FloatDivOp = Enums::FloatDiv;
const OpClass FloatSqrtOp = Enums::FloatSqrt;
const OpClass MemReadOp = Enums::MemRead;
const OpClass MemWriteOp = Enums::MemWrite;
const OpClass IprAccessOp = Enums::IprAccess;
const OpClass InstPrefetchOp = Enums::InstPrefetch;
const OpClass Num_OpClasses = Num_OpClass;

#endif // __CPU__OP_CLASS_HH__
