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
 *          Kevin T. Lim
 */

#ifndef __ALPHA_FAULTS_HH__
#define __ALPHA_FAULTS_HH__

#include "config/full_system.hh"
#include "sim/faults.hh"

#include "arch/alpha/pagetable.hh"

// The design of the "name" and "vect" functions is in sim/faults.hh

namespace AlphaISA
{

typedef const Addr FaultVect;

class AlphaFault : public FaultBase
{
  protected:
    virtual bool skipFaultingInstruction() {return false;}
    virtual bool setRestartAddress() {return true;}
  public:
#if FULL_SYSTEM
    void invoke(ThreadContext * tc);
#endif
    virtual FaultVect vect() = 0;
    virtual FaultStat & countStat() = 0;
};

class MachineCheckFault : public AlphaFault
{
  private:
    static FaultName _name;
    static FaultVect _vect;
    static FaultStat _count;
  public:
    FaultName name() const {return _name;}
    FaultVect vect() {return _vect;}
    FaultStat & countStat() {return _count;}
    bool isMachineCheckFault() const {return true;}
};

class AlignmentFault : public AlphaFault
{
  private:
    static FaultName _name;
    static FaultVect _vect;
    static FaultStat _count;
  public:
    FaultName name() const {return _name;}
    FaultVect vect() {return _vect;}
    FaultStat & countStat() {return _count;}
    bool isAlignmentFault() const {return true;}
};

static inline Fault genMachineCheckFault()
{
    return new MachineCheckFault;
}

class ResetFault : public AlphaFault
{
  private:
    static FaultName _name;
    static FaultVect _vect;
    static FaultStat _count;
  public:
    FaultName name() const {return _name;}
    FaultVect vect() {return _vect;}
    FaultStat & countStat() {return _count;}
};

class ArithmeticFault : public AlphaFault
{
  protected:
    bool skipFaultingInstruction() {return true;}
  private:
    static FaultName _name;
    static FaultVect _vect;
    static FaultStat _count;
  public:
    FaultName name() const {return _name;}
    FaultVect vect() {return _vect;}
    FaultStat & countStat() {return _count;}
#if FULL_SYSTEM
    void invoke(ThreadContext * tc);
#endif
};

class InterruptFault : public AlphaFault
{
  protected:
    bool setRestartAddress() {return false;}
  private:
    static FaultName _name;
    static FaultVect _vect;
    static FaultStat _count;
  public:
    FaultName name() const {return _name;}
    FaultVect vect() {return _vect;}
    FaultStat & countStat() {return _count;}
};

class DtbFault : public AlphaFault
{
  protected:
    AlphaISA::VAddr vaddr;
    uint32_t reqFlags;
    uint64_t flags;
  public:
    DtbFault(AlphaISA::VAddr _vaddr, uint32_t _reqFlags, uint64_t _flags)
        : vaddr(_vaddr), reqFlags(_reqFlags), flags(_flags)
    { }
    FaultName name() const = 0;
    FaultVect vect() = 0;
    FaultStat & countStat() = 0;
#if FULL_SYSTEM
    void invoke(ThreadContext * tc);
#endif
};

class NDtbMissFault : public DtbFault
{
  private:
    static FaultName _name;
    static FaultVect _vect;
    static FaultStat _count;
  public:
    NDtbMissFault(AlphaISA::VAddr vaddr, uint32_t reqFlags, uint64_t flags)
        : DtbFault(vaddr, reqFlags, flags)
    { }
    FaultName name() const {return _name;}
    FaultVect vect() {return _vect;}
    FaultStat & countStat() {return _count;}
#if !FULL_SYSTEM
    void invoke(ThreadContext * tc);
#endif
};

class PDtbMissFault : public DtbFault
{
  private:
    static FaultName _name;
    static FaultVect _vect;
    static FaultStat _count;
  public:
    PDtbMissFault(AlphaISA::VAddr vaddr, uint32_t reqFlags, uint64_t flags)
        : DtbFault(vaddr, reqFlags, flags)
    { }
    FaultName name() const {return _name;}
    FaultVect vect() {return _vect;}
    FaultStat & countStat() {return _count;}
};

class DtbPageFault : public DtbFault
{
  private:
    static FaultName _name;
    static FaultVect _vect;
    static FaultStat _count;
  public:
    DtbPageFault(AlphaISA::VAddr vaddr, uint32_t reqFlags, uint64_t flags)
        : DtbFault(vaddr, reqFlags, flags)
    { }
    FaultName name() const {return _name;}
    FaultVect vect() {return _vect;}
    FaultStat & countStat() {return _count;}
};

class DtbAcvFault : public DtbFault
{
  private:
    static FaultName _name;
    static FaultVect _vect;
    static FaultStat _count;
  public:
    DtbAcvFault(AlphaISA::VAddr vaddr, uint32_t reqFlags, uint64_t flags)
        : DtbFault(vaddr, reqFlags, flags)
    { }
    FaultName name() const {return _name;}
    FaultVect vect() {return _vect;}
    FaultStat & countStat() {return _count;}
};

class DtbAlignmentFault : public DtbFault
{
  private:
    static FaultName _name;
    static FaultVect _vect;
    static FaultStat _count;
  public:
    DtbAlignmentFault(AlphaISA::VAddr vaddr, uint32_t reqFlags, uint64_t flags)
        : DtbFault(vaddr, reqFlags, flags)
    { }
    FaultName name() const {return _name;}
    FaultVect vect() {return _vect;}
    FaultStat & countStat() {return _count;}
};

class ItbFault : public AlphaFault
{
  protected:
    Addr pc;
  public:
    ItbFault(Addr _pc)
        : pc(_pc)
    { }
    FaultName name() const = 0;
    FaultVect vect() = 0;
    FaultStat & countStat() = 0;
#if FULL_SYSTEM
    void invoke(ThreadContext * tc);
#endif
};

class ItbPageFault : public ItbFault
{
  private:
    static FaultName _name;
    static FaultVect _vect;
    static FaultStat _count;
  public:
    ItbPageFault(Addr pc)
        : ItbFault(pc)
    { }
    FaultName name() const {return _name;}
    FaultVect vect() {return _vect;}
    FaultStat & countStat() {return _count;}
#if !FULL_SYSTEM
    void invoke(ThreadContext * tc);
#endif
};

class ItbAcvFault : public ItbFault
{
  private:
    static FaultName _name;
    static FaultVect _vect;
    static FaultStat _count;
  public:
    ItbAcvFault(Addr pc)
        : ItbFault(pc)
    { }
    FaultName name() const {return _name;}
    FaultVect vect() {return _vect;}
    FaultStat & countStat() {return _count;}
};

class UnimplementedOpcodeFault : public AlphaFault
{
  private:
    static FaultName _name;
    static FaultVect _vect;
    static FaultStat _count;
  public:
    FaultName name() const {return _name;}
    FaultVect vect() {return _vect;}
    FaultStat & countStat() {return _count;}
};

class FloatEnableFault : public AlphaFault
{
  private:
    static FaultName _name;
    static FaultVect _vect;
    static FaultStat _count;
  public:
    FaultName name() const {return _name;}
    FaultVect vect() {return _vect;}
    FaultStat & countStat() {return _count;}
};

class PalFault : public AlphaFault
{
  protected:
    bool skipFaultingInstruction() {return true;}
  private:
    static FaultName _name;
    static FaultVect _vect;
    static FaultStat _count;
  public:
    FaultName name() const {return _name;}
    FaultVect vect() {return _vect;}
    FaultStat & countStat() {return _count;}
};

class IntegerOverflowFault : public AlphaFault
{
  private:
    static FaultName _name;
    static FaultVect _vect;
    static FaultStat _count;
  public:
    FaultName name() const {return _name;}
    FaultVect vect() {return _vect;}
    FaultStat & countStat() {return _count;}
};

} // AlphaISA namespace

#endif // __FAULTS_HH__
