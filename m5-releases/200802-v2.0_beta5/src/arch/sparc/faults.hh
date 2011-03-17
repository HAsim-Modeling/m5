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

#ifndef __SPARC_FAULTS_HH__
#define __SPARC_FAULTS_HH__

#include "config/full_system.hh"
#include "sim/faults.hh"

// The design of the "name" and "vect" functions is in sim/faults.hh

namespace SparcISA
{

typedef uint32_t TrapType;
typedef uint32_t FaultPriority;

class ITB;

class SparcFaultBase : public FaultBase
{
  public:
    enum PrivilegeLevel
    {
        U, User = U,
        P, Privileged = P,
        H, Hyperprivileged = H,
        NumLevels,
        SH = -1,
        ShouldntHappen = SH
    };
    struct FaultVals
    {
        const FaultName name;
        const TrapType trapType;
        const FaultPriority priority;
        const PrivilegeLevel nextPrivilegeLevel[NumLevels];
        FaultStat count;
    };
#if FULL_SYSTEM
    void invoke(ThreadContext * tc);
#endif
    virtual TrapType trapType() = 0;
    virtual FaultPriority priority() = 0;
    virtual FaultStat & countStat() = 0;
    virtual PrivilegeLevel getNextLevel(PrivilegeLevel current) = 0;
};

template<typename T>
class SparcFault : public SparcFaultBase
{
  protected:
    static FaultVals vals;
  public:
    FaultName name() const {return vals.name;}
    TrapType trapType() {return vals.trapType;}
    FaultPriority priority() {return vals.priority;}
    FaultStat & countStat() {return vals.count;}
    PrivilegeLevel getNextLevel(PrivilegeLevel current)
    {
        return vals.nextPrivilegeLevel[current];
    }
};

class PowerOnReset : public SparcFault<PowerOnReset>
{
    void invoke(ThreadContext * tc);
};

class WatchDogReset : public SparcFault<WatchDogReset> {};

class ExternallyInitiatedReset : public SparcFault<ExternallyInitiatedReset> {};

class SoftwareInitiatedReset : public SparcFault<SoftwareInitiatedReset> {};

class REDStateException : public SparcFault<REDStateException> {};

class StoreError : public SparcFault<StoreError> {};

class InstructionAccessException : public SparcFault<InstructionAccessException> {};

//class InstructionAccessMMUMiss : public SparcFault<InstructionAccessMMUMiss> {};

class InstructionAccessError : public SparcFault<InstructionAccessError> {};

class IllegalInstruction : public SparcFault<IllegalInstruction> {};

class PrivilegedOpcode : public SparcFault<PrivilegedOpcode> {};

//class UnimplementedLDD : public SparcFault<UnimplementedLDD> {};

//class UnimplementedSTD : public SparcFault<UnimplementedSTD> {};

class FpDisabled : public SparcFault<FpDisabled> {};

class FpExceptionIEEE754 : public SparcFault<FpExceptionIEEE754> {};

class FpExceptionOther : public SparcFault<FpExceptionOther> {};

class TagOverflow : public SparcFault<TagOverflow> {};

class CleanWindow : public SparcFault<CleanWindow> {};

class DivisionByZero : public SparcFault<DivisionByZero> {};

class InternalProcessorError :
    public SparcFault<InternalProcessorError>
{
  public:
    bool isMachineCheckFault() const {return true;}
};

class InstructionInvalidTSBEntry : public SparcFault<InstructionInvalidTSBEntry> {};

class DataInvalidTSBEntry : public SparcFault<DataInvalidTSBEntry> {};

class DataAccessException : public SparcFault<DataAccessException> {};

//class DataAccessMMUMiss : public SparcFault<DataAccessMMUMiss> {};

class DataAccessError : public SparcFault<DataAccessError> {};

class DataAccessProtection : public SparcFault<DataAccessProtection> {};

class MemAddressNotAligned :
    public SparcFault<MemAddressNotAligned>
{
  public:
    bool isAlignmentFault() const {return true;}
};

class LDDFMemAddressNotAligned : public SparcFault<LDDFMemAddressNotAligned> {};

class STDFMemAddressNotAligned : public SparcFault<STDFMemAddressNotAligned> {};

class PrivilegedAction : public SparcFault<PrivilegedAction> {};

class LDQFMemAddressNotAligned : public SparcFault<LDQFMemAddressNotAligned> {};

class STQFMemAddressNotAligned : public SparcFault<STQFMemAddressNotAligned> {};

class InstructionRealTranslationMiss :
    public SparcFault<InstructionRealTranslationMiss> {};

class DataRealTranslationMiss : public SparcFault<DataRealTranslationMiss> {};

//class AsyncDataError : public SparcFault<AsyncDataError> {};

template <class T>
class EnumeratedFault : public SparcFault<T>
{
  protected:
    uint32_t _n;
  public:
    EnumeratedFault(uint32_t n) : SparcFault<T>(), _n(n) {}
    TrapType trapType() {return SparcFault<T>::trapType() + _n;}
};

class InterruptLevelN : public EnumeratedFault<InterruptLevelN>
{
  public:
    InterruptLevelN(uint32_t n) : EnumeratedFault<InterruptLevelN>(n) {;}
    FaultPriority priority() {return 3200 - _n*100;}
};

class HstickMatch : public SparcFault<HstickMatch> {};

class TrapLevelZero : public SparcFault<TrapLevelZero> {};

class InterruptVector : public SparcFault<InterruptVector> {};

class PAWatchpoint : public SparcFault<PAWatchpoint> {};

class VAWatchpoint : public SparcFault<VAWatchpoint> {};

class FastInstructionAccessMMUMiss :
    public SparcFault<FastInstructionAccessMMUMiss>
{
#if !FULL_SYSTEM
  protected:
    Addr vaddr;
  public:
    FastInstructionAccessMMUMiss(Addr addr) : vaddr(addr)
    {}
    void invoke(ThreadContext * tc);
#endif
};

class FastDataAccessMMUMiss : public SparcFault<FastDataAccessMMUMiss>
{
#if !FULL_SYSTEM
  protected:
    Addr vaddr;
  public:
    FastDataAccessMMUMiss(Addr addr) : vaddr(addr)
    {}
    void invoke(ThreadContext * tc);
#endif
};

class FastDataAccessProtection : public SparcFault<FastDataAccessProtection> {};

class InstructionBreakpoint : public SparcFault<InstructionBreakpoint> {};

class CpuMondo : public SparcFault<CpuMondo> {};

class DevMondo : public SparcFault<DevMondo> {};

class ResumableError : public SparcFault<ResumableError> {};

class SpillNNormal : public EnumeratedFault<SpillNNormal>
{
  public:
    SpillNNormal(uint32_t n) : EnumeratedFault<SpillNNormal>(n) {;}
    //These need to be handled specially to enable spill traps in SE
#if !FULL_SYSTEM
    void invoke(ThreadContext * tc);
#endif
};

class SpillNOther : public EnumeratedFault<SpillNOther>
{
  public:
    SpillNOther(uint32_t n) : EnumeratedFault<SpillNOther>(n) {;}
};

class FillNNormal : public EnumeratedFault<FillNNormal>
{
  public:
    FillNNormal(uint32_t n) : EnumeratedFault<FillNNormal>(n) {;}
    //These need to be handled specially to enable fill traps in SE
#if !FULL_SYSTEM
    void invoke(ThreadContext * tc);
#endif
};

class FillNOther : public EnumeratedFault<FillNOther>
{
  public:
    FillNOther(uint32_t n) : EnumeratedFault<FillNOther>(n) {;}
};

class TrapInstruction : public EnumeratedFault<TrapInstruction>
{
  public:
    TrapInstruction(uint32_t n) : EnumeratedFault<TrapInstruction>(n) {;}
    //In SE, trap instructions are requesting services from the OS.
#if !FULL_SYSTEM
    void invoke(ThreadContext * tc);
#endif
};

static inline Fault genMachineCheckFault()
{
    return new InternalProcessorError;
}


} // SparcISA namespace

#endif // __SPARC_FAULTS_HH__
