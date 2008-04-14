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
 */

#ifndef __CPU_THREAD_STATE_HH__
#define __CPU_THREAD_STATE_HH__

#include "arch/types.hh"
#include "cpu/profile.hh"
#include "cpu/thread_context.hh"

#if !FULL_SYSTEM
#include "mem/mem_object.hh"
#include "sim/process.hh"
#endif

#if FULL_SYSTEM
class EndQuiesceEvent;
class FunctionProfile;
class ProfileNode;
namespace TheISA {
    namespace Kernel {
        class Statistics;
    };
};
#endif

class BaseCPU;
class Checkpoint;
class Port;
class TranslatingPort;

/**
 *  Struct for holding general thread state that is needed across CPU
 *  models.  This includes things such as pointers to the process,
 *  memory, quiesce events, and certain stats.  This can be expanded
 *  to hold more thread-specific stats within it.
 */
struct ThreadState {
    typedef ThreadContext::Status Status;

#if FULL_SYSTEM
    ThreadState(BaseCPU *cpu, int _cpuId, int _tid);
#else
    ThreadState(BaseCPU *cpu, int _cpuId, int _tid, Process *_process,
                short _asid);
#endif

    ~ThreadState();

    void serialize(std::ostream &os);

    void unserialize(Checkpoint *cp, const std::string &section);

    void setCpuId(int id) { cpuId = id; }

    int readCpuId() { return cpuId; }

    void setTid(int id) { tid = id; }

    int readTid() { return tid; }

    Tick readLastActivate() { return lastActivate; }

    Tick readLastSuspend() { return lastSuspend; }

#if FULL_SYSTEM
    void connectMemPorts();

    void connectPhysPort();

    void connectVirtPort();

    void dumpFuncProfile();

    EndQuiesceEvent *getQuiesceEvent() { return quiesceEvent; }

    void profileClear();

    void profileSample();

    TheISA::Kernel::Statistics *getKernelStats() { return kernelStats; }

    FunctionalPort *getPhysPort() { return physPort; }

    void setPhysPort(FunctionalPort *port) { physPort = port; }

    VirtualPort *getVirtPort(ThreadContext *tc = NULL) { return virtPort; }

    void setVirtPort(VirtualPort *port) { virtPort = port; }
#else
    Process *getProcessPtr() { return process; }

    TranslatingPort *getMemPort();

    void setMemPort(TranslatingPort *_port) { port = _port; }

    int getInstAsid() { return asid; }
    int getDataAsid() { return asid; }
#endif

    /** Sets the current instruction being committed. */
    void setInst(TheISA::MachInst _inst) { inst = _inst; }

    /** Returns the current instruction being committed. */
    TheISA::MachInst getInst() { return inst; }

    /** Reads the number of instructions functionally executed and
     * committed.
     */
    Counter readFuncExeInst() { return funcExeInst; }

    /** Sets the total number of instructions functionally executed
     * and committed.
     */
    void setFuncExeInst(Counter new_val) { funcExeInst = new_val; }

    /** Returns the status of this thread. */
    Status status() const { return _status; }

    /** Sets the status of this thread. */
    void setStatus(Status new_status) { _status = new_status; }

  public:
    /** Connects port to the functional port of the memory object
     * below the CPU. */
    void connectToMemFunc(Port *port);

    /** Number of instructions committed. */
    Counter numInst;
    /** Stat for number instructions committed. */
    Stats::Scalar<> numInsts;
    /** Stat for number of memory references. */
    Stats::Scalar<> numMemRefs;

    /** Number of simulated loads, used for tracking events based on
     * the number of loads committed.
     */
    Counter numLoad;

    /** The number of simulated loads committed prior to this run. */
    Counter startNumLoad;

  protected:
    ThreadContext::Status _status;

    // Pointer to the base CPU.
    BaseCPU *baseCpu;

    // ID of this context w.r.t. the System or Process object to which
    // it belongs.  For full-system mode, this is the system CPU ID.
    int cpuId;

    // Index of hardware thread context on the CPU that this represents.
    int tid;

  public:
    /** Last time activate was called on this thread. */
    Tick lastActivate;

    /** Last time suspend was called on this thread. */
    Tick lastSuspend;

#if FULL_SYSTEM
  public:
    FunctionProfile *profile;
    ProfileNode *profileNode;
    Addr profilePC;
    EndQuiesceEvent *quiesceEvent;

    TheISA::Kernel::Statistics *kernelStats;
  protected:
    /** A functional port outgoing only for functional accesses to physical
     * addresses.*/
    FunctionalPort *physPort;

    /** A functional port, outgoing only, for functional accesse to virtual
     * addresses. That doen't require execution context information */
    VirtualPort *virtPort;
#else
    TranslatingPort *port;

    Process *process;

    // Address space ID.  Note that this is used for TIMING cache
    // simulation only; all functional memory accesses should use
    // one of the FunctionalMemory pointers above.
    short asid;

#endif

    /** Current instruction the thread is committing.  Only set and
     * used for DTB faults currently.
     */
    TheISA::MachInst inst;

    /** The current microcode pc for the currently executing macro
     * operation.
     */
    MicroPC microPC;

    /** The next microcode pc for the currently executing macro
     * operation.
     */
    MicroPC nextMicroPC;

  public:
    /**
     * Temporary storage to pass the source address from copy_load to
     * copy_store.
     * @todo Remove this temporary when we have a better way to do it.
     */
    Addr copySrcAddr;
    /**
     * Temp storage for the physical source address of a copy.
     * @todo Remove this temporary when we have a better way to do it.
     */
    Addr copySrcPhysAddr;

    /*
     * number of executed instructions, for matching with syscall trace
     * points in EIO files.
     */
    Counter funcExeInst;

    //
    // Count failed store conditionals so we can warn of apparent
    // application deadlock situations.
    unsigned storeCondFailures;
};

#endif // __CPU_THREAD_STATE_HH__
