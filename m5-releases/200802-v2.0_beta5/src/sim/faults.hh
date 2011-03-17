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
 *          Gabe M. Black
 */

#ifndef __FAULTS_HH__
#define __FAULTS_HH__

#include "base/refcnt.hh"
#include "sim/stats.hh"
#include "config/full_system.hh"

class ThreadContext;
class FaultBase;
typedef RefCountingPtr<FaultBase> Fault;

typedef const char * FaultName;
typedef Stats::Scalar<> FaultStat;

// Each class has it's name statically define in _name,
// and has a virtual function to access it's name.
// The function is necessary because otherwise, all objects
// which are being accessed cast as a FaultBase * (namely
// all faults returned using the Fault type) will use the
// generic FaultBase name.

class FaultBase : public RefCounted
{
  public:
    virtual FaultName name() const = 0;
    virtual void invoke(ThreadContext * tc);
//    template<typename T>
//    bool isA() {return dynamic_cast<T *>(this);}
    virtual bool isMachineCheckFault() const {return false;}
    virtual bool isAlignmentFault() const {return false;}
};

FaultBase * const NoFault = 0;

class UnimpFault : public FaultBase
{
  private:
    std::string panicStr;
  public:
    UnimpFault(std::string _str)
        : panicStr(_str)
    { }

    FaultName name() const {return "Unimplemented simulator feature";}
    void invoke(ThreadContext * tc);
};

#if !FULL_SYSTEM
class GenericPageTableFault : public FaultBase
{
  private:
    Addr vaddr;
  public:
    FaultName name() const {return "Generic page table fault";}
    GenericPageTableFault(Addr va) : vaddr(va) {}
    void invoke(ThreadContext * tc);
};

class GenericAlignmentFault : public FaultBase
{
  private:
    Addr vaddr;
  public:
    FaultName name() const {return "Generic alignment fault";}
    GenericAlignmentFault(Addr va) : vaddr(va) {}
    void invoke(ThreadContext * tc);
};
#endif

#endif // __FAULTS_HH__
