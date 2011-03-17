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
 * Authors: Gabe Black
 */

#ifndef __SIM_TLB_HH__
#define __SIM_TLB_HH__

#include "base/misc.hh"
#include "mem/request.hh"
#include "sim/faults.hh"
#include "sim/sim_object.hh"

class ThreadContext;
class Packet;

class BaseTLB : public SimObject
{
  protected:
    BaseTLB(const Params *p) : SimObject(p)
    {}

  public:
    virtual void demapPage(Addr vaddr, uint64_t asn) = 0;
};

class GenericTLB : public BaseTLB
{
  protected:
    GenericTLB(const Params *p) : BaseTLB(p)
    {}

  public:
    void demapPage(Addr vaddr, uint64_t asn);

    Fault translate(RequestPtr req, ThreadContext *tc, bool=false);
};

#endif // __ARCH_SPARC_TLB_HH__
