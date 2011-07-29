/*
 * Copyright (c) 2002, 2003, 2004, 2005
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
 * Authors: Steven K. Reinhardt
 *          Nathan L. Binkert
 */

#ifndef __CPU_BASE_HH__
#define __CPU_BASE_HH__

#include <vector>

#include "arch/isa_traits.hh"
#include "base/statistics.hh"
#include "config/full_system.hh"
#include "sim/eventq.hh"
#include "sim/insttracer.hh"
#include "mem/mem_object.hh"

#if FULL_SYSTEM
#include "arch/interrupts.hh"
#endif

class BranchPred;
class CheckerCPU;
class ThreadContext;
class System;
class Port;

namespace TheISA
{
    class Predecoder;
}

class CPUProgressEvent : public Event
{
  protected:
    Tick interval;
    Counter lastNumInst;
    BaseCPU *cpu;

  public:
    CPUProgressEvent(EventQueue *q, Tick ival, BaseCPU *_cpu);

    void process();

    virtual const char *description() const;
};

class BaseCPU : public MemObject
{
  protected:
    // CPU's clock period in terms of the number of ticks of curTime.
    Tick clock;
    // @todo remove me after debugging with legion done
    Tick instCnt;

  public:
//    Tick currentTick;
    inline Tick frequency() const { return Clock::Frequency / clock; }
    inline Tick ticks(int numCycles) const { return clock * numCycles; }
    inline Tick curCycle() const { return curTick / clock; }
    inline Tick tickToCycles(Tick val) const { return val / clock; }
    // @todo remove me after debugging with legion done
    Tick instCount() { return instCnt; }

    /** The next cycle the CPU should be scheduled, given a cache
     * access or quiesce event returning on this cycle.  This function
     * may return curTick if the CPU should run on the current cycle.
     */
    Tick nextCycle();

    /** The next cycle the CPU should be scheduled, given a cache
     * access or quiesce event returning on the given Tick.  This
     * function may return curTick if the CPU should run on the
     * current cycle.
     * @param begin_tick The tick that the event is completing on.
     */
    Tick nextCycle(Tick begin_tick);

#if FULL_SYSTEM
  protected:
//    uint64_t interrupts[TheISA::NumInterruptLevels];
//    uint64_t intstatus;
    TheISA::Interrupts interrupts;

  public:
    virtual void post_interrupt(int int_num, int index);
    virtual void clear_interrupt(int int_num, int index);
    virtual void clear_interrupts();
    virtual uint64_t get_interrupts(int int_num);

    bool check_interrupts(ThreadContext * tc) const
    { return interrupts.check_interrupts(tc); }

    class ProfileEvent : public Event
    {
      private:
        BaseCPU *cpu;
        int interval;

      public:
        ProfileEvent(BaseCPU *cpu, int interval);
        void process();
    };
    ProfileEvent *profileEvent;
#endif

  protected:
    std::vector<ThreadContext *> threadContexts;
    std::vector<TheISA::Predecoder *> predecoders;

    Trace::InstTracer * tracer;

  public:

    /// Provide access to the tracer pointer
    Trace::InstTracer * getTracer() { return tracer; }

    /// Notify the CPU that the indicated context is now active.  The
    /// delay parameter indicates the number of ticks to wait before
    /// executing (typically 0 or 1).
    virtual void activateContext(int thread_num, int delay) {}

    /// Notify the CPU that the indicated context is now suspended.
    virtual void suspendContext(int thread_num) {}

    /// Notify the CPU that the indicated context is now deallocated.
    virtual void deallocateContext(int thread_num) {}

    /// Notify the CPU that the indicated context is now halted.
    virtual void haltContext(int thread_num) {}

   /// Given a Thread Context pointer return the thread num
   int findContext(ThreadContext *tc);

   /// Given a thread num get tho thread context for it
   ThreadContext *getContext(int tn) { return threadContexts[tn]; }

  public:
    struct Params
    {
        std::string name;
        int numberOfThreads;
        bool deferRegistration;
        Counter max_insts_any_thread;
        Counter max_insts_all_threads;
        Counter max_loads_any_thread;
        Counter max_loads_all_threads;
        Tick clock;
        bool functionTrace;
        Tick functionTraceStart;
        System *system;
        int cpu_id;
        Trace::InstTracer * tracer;

        Tick phase;
#if FULL_SYSTEM
        Tick profile;

        bool do_statistics_insts;
        bool do_checkpoint_insts;
        bool do_quiesce;
#endif
        Tick progress_interval;
        BaseCPU *checker;

        TheISA::CoreSpecific coreParams; //ISA-Specific Params That Set Up State in Core

        Params();
    };

    const Params *params;

    BaseCPU(Params *params);
    virtual ~BaseCPU();

    virtual void init();
    virtual void startup();
    virtual void regStats();

    virtual void activateWhenReady(int tid) {};

    void registerThreadContexts();

    /// Prepare for another CPU to take over execution.  When it is
    /// is ready (drained pipe) it signals the sampler.
    virtual void switchOut();

    /// Take over execution from the given CPU.  Used for warm-up and
    /// sampling.
    virtual void takeOverFrom(BaseCPU *, Port *ic, Port *dc);

    /**
     *  Number of threads we're actually simulating (<= SMT_MAX_THREADS).
     * This is a constant for the duration of the simulation.
     */
    int number_of_threads;

    /**
     * Vector of per-thread instruction-based event queues.  Used for
     * scheduling events based on number of instructions committed by
     * a particular thread.
     */
    EventQueue **comInstEventQueue;

    /**
     * Vector of per-thread load-based event queues.  Used for
     * scheduling events based on number of loads committed by
     *a particular thread.
     */
    EventQueue **comLoadEventQueue;

    System *system;

    Tick phase;

#if FULL_SYSTEM
    /**
     * Serialize this object to the given output stream.
     * @param os The stream to serialize to.
     */
    virtual void serialize(std::ostream &os);

    /**
     * Reconstruct the state of this object from a checkpoint.
     * @param cp The checkpoint use.
     * @param section The section name of this object
     */
    virtual void unserialize(Checkpoint *cp, const std::string &section);

#endif

    /**
     * Return pointer to CPU's branch predictor (NULL if none).
     * @return Branch predictor pointer.
     */
    virtual BranchPred *getBranchPred() { return NULL; };

    virtual Counter totalInstructions() const { return 0; }

    // Function tracing
  private:
    bool functionTracingEnabled;
    std::ostream *functionTraceStream;
    Addr currentFunctionStart;
    Addr currentFunctionEnd;
    Tick functionEntryTick;
    void enableFunctionTrace();
    void traceFunctionsInternal(Addr pc);

  protected:
    void traceFunctions(Addr pc)
    {
        if (functionTracingEnabled)
            traceFunctionsInternal(pc);
    }

  private:
    static std::vector<BaseCPU *> cpuList;   //!< Static global cpu list

  public:
    static int numSimulatedCPUs() { return cpuList.size(); }
    static Counter numSimulatedInstructions()
    {
        Counter total = 0;

        int size = cpuList.size();
        for (int i = 0; i < size; ++i)
            total += cpuList[i]->totalInstructions();

        return total;
    }

  public:
    // Number of CPU cycles simulated
    Stats::Scalar<> numCycles;
};

#endif // __CPU_BASE_HH__