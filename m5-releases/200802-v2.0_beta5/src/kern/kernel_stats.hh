/*
 * Copyright (c) 2004, 2005
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
 * Authors: Lisa R. Hsu
 *          Nathan L. Binkert
 */

#ifndef __KERNEL_STATS_HH__
#define __KERNEL_STATS_HH__

#include <string>

#include "cpu/static_inst.hh"
#include "sim/serialize.hh"

class BaseCPU;
class ThreadContext;
class FnEvent;
// What does kernel stats expect is included?
class System;

namespace Kernel {

class Statistics : public Serializable
{
  protected:
    std::string myname;

  protected:
    Stats::Scalar<> _arm;
    Stats::Scalar<> _quiesce;
    Stats::Scalar<> _hwrei;

    Stats::Vector<> _iplCount;
    Stats::Vector<> _iplGood;
    Stats::Vector<> _iplTicks;
    Stats::Formula _iplUsed;

    Stats::Vector<> _syscall;
//    Stats::Vector<> _faults;

  private:
    int iplLast;
    Tick iplLastTick;

  public:
    Statistics(System *system);
    virtual ~Statistics() {}

    const std::string name() const { return myname; }
    void regStats(const std::string &name);

  public:
    void arm() { _arm++; }
    void quiesce() { _quiesce++; }
    void swpipl(int ipl);

  public:
    virtual void serialize(std::ostream &os);
    virtual void unserialize(Checkpoint *cp, const std::string &section);
};

/* end namespace Kernel */ }

#endif // __KERNEL_STATS_HH__
