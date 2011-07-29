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

#ifndef __CPU_FUNC_UNIT_HH__
#define __CPU_FUNC_UNIT_HH__

#include <bitset>
#include <string>
#include <vector>

#include "cpu/op_class.hh"
#include "params/OpDesc.hh"
#include "params/FUDesc.hh"
#include "sim/sim_object.hh"

////////////////////////////////////////////////////////////////////////////
//
//  Structures used ONLY during the initialization phase...
//
//
//

struct OpDesc : public SimObject
{
    OpClass opClass;
    unsigned    opLat;
    unsigned    issueLat;

    OpDesc(const OpDescParams *p)
        : SimObject(p), opClass(p->opClass), opLat(p->opLat),
          issueLat(p->issueLat) {};
};

struct FUDesc : public SimObject
{
    std::vector<OpDesc *> opDescList;
    unsigned         number;

    FUDesc(const FUDescParams *p)
        : SimObject(p), opDescList(p->opList), number(p->count) {};
};

typedef std::vector<OpDesc *>::const_iterator OPDDiterator;
typedef std::vector<FUDesc *>::const_iterator FUDDiterator;




////////////////////////////////////////////////////////////////////////////
//
//  The actual FU object
//
//
//
class FuncUnit
{
  private:
    unsigned opLatencies[Num_OpClasses];
    unsigned issueLatencies[Num_OpClasses];
    std::bitset<Num_OpClasses> capabilityList;

  public:
    FuncUnit();
    FuncUnit(const FuncUnit &fu);

    std::string name;

    void addCapability(OpClass cap, unsigned oplat, unsigned issuelat);

    bool provides(OpClass capability);
    std::bitset<Num_OpClasses> capabilities();

    unsigned &opLatency(OpClass capability);
    unsigned issueLatency(OpClass capability);
};

#endif // __FU_POOL_HH__