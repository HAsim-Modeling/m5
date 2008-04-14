/*
 * Copyright (c) 2002, 2003, 2004, 2005, 2006
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
 * Authors: Steven E. Raasch
 */

#include <sstream>

#include "base/misc.hh"
#include "cpu/func_unit.hh"

using namespace std;


////////////////////////////////////////////////////////////////////////////
//
//  The funciton unit
//
FuncUnit::FuncUnit()
{
    capabilityList.reset();
}


//  Copy constructor
FuncUnit::FuncUnit(const FuncUnit &fu)
{

    for (int i = 0; i < Num_OpClasses; ++i) {
        opLatencies[i] = fu.opLatencies[i];
        issueLatencies[i] = fu.issueLatencies[i];
    }

    capabilityList = fu.capabilityList;
}


void
FuncUnit::addCapability(OpClass cap, unsigned oplat, unsigned issuelat)
{
    if (issuelat == 0 || oplat == 0)
        panic("FuncUnit:  you don't really want a zero-cycle latency do you?");

    capabilityList.set(cap);

    opLatencies[cap] = oplat;
    issueLatencies[cap] = issuelat;
}

bool
FuncUnit::provides(OpClass capability)
{
    return capabilityList[capability];
}

bitset<Num_OpClasses>
FuncUnit::capabilities()
{
    return capabilityList;
}

unsigned &
FuncUnit::opLatency(OpClass cap)
{
    return opLatencies[cap];
}

unsigned
FuncUnit::issueLatency(OpClass capability)
{
    return issueLatencies[capability];
}

////////////////////////////////////////////////////////////////////////////
//
//  The SimObjects we use to get the FU information into the simulator
//
////////////////////////////////////////////////////////////////////////////

//
//  We use 2 objects to specify this data in the INI file:
//    (1) OpDesc - Describes the operation class & latencies
//                   (multiple OpDesc objects can refer to the same
//                   operation classes)
//    (2) FUDesc - Describes the operations available in the unit &
//                   the number of these units
//
//


//
//  The operation-class description object
//
OpDesc *
OpDescParams::create()
{
    return new OpDesc(this);
}

//
//  The FuDesc object
//
FUDesc *
FUDescParams::create()
{
    return new FUDesc(this);
}
