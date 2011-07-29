/*
 * Copyright (c) 2004, 2005, 2006
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
 *          Korey L. Sewell
 */

#ifndef __CPU_O3_COMMIT_HH__
#define __CPU_O3_COMMIT_HH__

#include "base/statistics.hh"
#include "base/timebuf.hh"
#include "cpu/exetrace.hh"
#include "cpu/inst_seq.hh"

template <class>
class O3ThreadState;

/**
 * DefaultCommit handles single threaded and SMT commit. Its width is
 * specified by the parameters; each cycle it tries to commit that
 * many instructions. The SMT policy decides which thread it tries to
 * commit instructions from. Non- speculative instructions must reach
 * the head of the ROB before they are ready to execute; once they
 * reach the head, commit will broadcast the instruction's sequence
 * number to the previous stages so that they can issue/ execute the
 * instruction. Only one non-speculative instruction is handled per
 * cycle. Commit is responsible for handling all back-end initiated
 * redirects.  It receives the redirect, and then broadcasts it to all
 * stages, indicating the sequence number they should squash until,
 * and any necessary branch misprediction information as well. It
 * priortizes redirects by instruction's age, only broadcasting a
 * redirect if it corresponds to an instruction that should currently
 * be in the ROB. This is done by tracking the sequence number of the
 * youngest instruction in the ROB, which gets updated to any
 * squashing instruction's sequence number, and only broadcasting a
 * redirect if it corresponds to an older instruction. Commit also
 * supports multiple cycle squashing, to model a ROB that can only
 * remove a certain number of instructions per cycle.
 */
template<class Impl>
class DefaultCommit
{
  public:
    // Typedefs from the Impl.
    typedef typename Impl::O3CPU O3CPU;
    typedef typename Impl::DynInstPtr DynInstPtr;
    typedef typename Impl::Params Params;
    typedef typename Impl::CPUPol CPUPol;

    typedef typename CPUPol::RenameMap RenameMap;
    typedef typename CPUPol::ROB ROB;

    typedef typename CPUPol::TimeStruct TimeStruct;
    typedef typename CPUPol::FetchStruct FetchStruct;
    typedef typename CPUPol::IEWStruct IEWStruct;
    typedef typename CPUPol::RenameStruct RenameStruct;

    typedef typename CPUPol::Fetch Fetch;
    typedef typename CPUPol::IEW IEW;

    typedef O3ThreadState<Impl> Thread;

    /** Event class used to schedule a squash due to a trap (fault or
     * interrupt) to happen on a specific cycle.
     */
    class TrapEvent : public Event {
      private:
        DefaultCommit<Impl> *commit;
        unsigned tid;

      public:
        TrapEvent(DefaultCommit<Impl> *_commit, unsigned _tid);

        void process();
        const char *description() const;
    };

    /** Overall commit status. Used to determine if the CPU can deschedule
     * itself due to a lack of activity.
     */
    enum CommitStatus{
        Active,
        Inactive
    };

    /** Individual thread status. */
    enum ThreadStatus {
        Running,
        Idle,
        ROBSquashing,
        TrapPending,
        FetchTrapPending
    };

    /** Commit policy for SMT mode. */
    enum CommitPolicy {
        Aggressive,
        RoundRobin,
        OldestReady
    };

  private:
    /** Overall commit status. */
    CommitStatus _status;
    /** Next commit status, to be set at the end of the cycle. */
    CommitStatus _nextStatus;
    /** Per-thread status. */
    ThreadStatus commitStatus[Impl::MaxThreads];
    /** Commit policy used in SMT mode. */
    CommitPolicy commitPolicy;

  public:
    /** Construct a DefaultCommit with the given parameters. */
    DefaultCommit(O3CPU *_cpu, Params *params);

    /** Returns the name of the DefaultCommit. */
    std::string name() const;

    /** Registers statistics. */
    void regStats();

    /** Sets the list of threads. */
    void setThreads(std::vector<Thread *> &threads);

    /** Sets the main time buffer pointer, used for backwards communication. */
    void setTimeBuffer(TimeBuffer<TimeStruct> *tb_ptr);

    void setFetchQueue(TimeBuffer<FetchStruct> *fq_ptr);

    /** Sets the pointer to the queue coming from rename. */
    void setRenameQueue(TimeBuffer<RenameStruct> *rq_ptr);

    /** Sets the pointer to the queue coming from IEW. */
    void setIEWQueue(TimeBuffer<IEWStruct> *iq_ptr);

    /** Sets the pointer to the IEW stage. */
    void setIEWStage(IEW *iew_stage);

    /** Skid buffer between rename and commit. */
    std::queue<DynInstPtr> skidBuffer;

    /** The pointer to the IEW stage. Used solely to ensure that
     * various events (traps, interrupts, syscalls) do not occur until
     * all stores have written back.
     */
    IEW *iewStage;

    /** Sets pointer to list of active threads. */
    void setActiveThreads(std::list<unsigned> *at_ptr);

    /** Sets pointer to the commited state rename map. */
    void setRenameMap(RenameMap rm_ptr[Impl::MaxThreads]);

    /** Sets pointer to the ROB. */
    void setROB(ROB *rob_ptr);

    /** Initializes stage by sending back the number of free entries. */
    void initStage();

    /** Initializes the draining of commit. */
    bool drain();

    /** Resumes execution after draining. */
    void resume();

    /** Completes the switch out of commit. */
    void switchOut();

    /** Takes over from another CPU's thread. */
    void takeOverFrom();

    /** Ticks the commit stage, which tries to commit instructions. */
    void tick();

    /** Handles any squashes that are sent from IEW, and adds instructions
     * to the ROB and tries to commit instructions.
     */
    void commit();

    /** Returns the number of free ROB entries for a specific thread. */
    unsigned numROBFreeEntries(unsigned tid);

    /** Generates an event to schedule a squash due to a trap. */
    void generateTrapEvent(unsigned tid);

    /** Records that commit needs to initiate a squash due to an
     * external state update through the TC.
     */
    void generateTCEvent(unsigned tid);

  private:
    /** Updates the overall status of commit with the nextStatus, and
     * tell the CPU if commit is active/inactive.
     */
    void updateStatus();

    /** Sets the next status based on threads' statuses, which becomes the
     * current status at the end of the cycle.
     */
    void setNextStatus();

    /** Checks if the ROB is completed with squashing. This is for the case
     * where the ROB can take multiple cycles to complete squashing.
     */
    bool robDoneSquashing();

    /** Returns if any of the threads have the number of ROB entries changed
     * on this cycle. Used to determine if the number of free ROB entries needs
     * to be sent back to previous stages.
     */
    bool changedROBEntries();

    /** Squashes all in flight instructions. */
    void squashAll(unsigned tid);

    /** Handles squashing due to a trap. */
    void squashFromTrap(unsigned tid);

    /** Handles squashing due to an TC write. */
    void squashFromTC(unsigned tid);

#if FULL_SYSTEM
    /** Handles processing an interrupt. */
    void handleInterrupt();
#endif // FULL_SYSTEM

    /** Commits as many instructions as possible. */
    void commitInsts();

    /** Tries to commit the head ROB instruction passed in.
     * @param head_inst The instruction to be committed.
     */
    bool commitHead(DynInstPtr &head_inst, unsigned inst_num);

    /** Gets instructions from rename and inserts them into the ROB. */
    void getInsts();

    /** Insert all instructions from rename into skidBuffer */
    void skidInsert();

    /** Marks completed instructions using information sent from IEW. */
    void markCompletedInsts();

    /** Gets the thread to commit, based on the SMT policy. */
    int getCommittingThread();

    /** Returns the thread ID to use based on a round robin policy. */
    int roundRobin();

    /** Returns the thread ID to use based on an oldest instruction policy. */
    int oldestReady();

  public:
    /** Returns the PC of the head instruction of the ROB.
     * @todo: Probably remove this function as it returns only thread 0.
     */
    Addr readPC() { return PC[0]; }

    /** Returns the PC of a specific thread. */
    Addr readPC(unsigned tid) { return PC[tid]; }

    /** Sets the PC of a specific thread. */
    void setPC(Addr val, unsigned tid) { PC[tid] = val; }

    /** Reads the micro PC of a specific thread. */
    Addr readMicroPC(unsigned tid) { return microPC[tid]; }

    /** Sets the micro PC of a specific thread */
    void setMicroPC(Addr val, unsigned tid) { microPC[tid] = val; }

    /** Reads the next PC of a specific thread. */
    Addr readNextPC(unsigned tid) { return nextPC[tid]; }

    /** Sets the next PC of a specific thread. */
    void setNextPC(Addr val, unsigned tid) { nextPC[tid] = val; }

    /** Reads the next NPC of a specific thread. */
    Addr readNextNPC(unsigned tid) { return nextNPC[tid]; }

    /** Sets the next NPC of a specific thread. */
    void setNextNPC(Addr val, unsigned tid) { nextNPC[tid] = val; }

    /** Reads the micro PC of a specific thread. */
    Addr readNextMicroPC(unsigned tid) { return nextMicroPC[tid]; }

    /** Sets the micro PC of a specific thread */
    void setNextMicroPC(Addr val, unsigned tid) { nextMicroPC[tid] = val; }

  private:
    /** Time buffer interface. */
    TimeBuffer<TimeStruct> *timeBuffer;

    /** Wire to write information heading to previous stages. */
    typename TimeBuffer<TimeStruct>::wire toIEW;

    /** Wire to read information from IEW (for ROB). */
    typename TimeBuffer<TimeStruct>::wire robInfoFromIEW;

    TimeBuffer<FetchStruct> *fetchQueue;

    typename TimeBuffer<FetchStruct>::wire fromFetch;

    /** IEW instruction queue interface. */
    TimeBuffer<IEWStruct> *iewQueue;

    /** Wire to read information from IEW queue. */
    typename TimeBuffer<IEWStruct>::wire fromIEW;

    /** Rename instruction queue interface, for ROB. */
    TimeBuffer<RenameStruct> *renameQueue;

    /** Wire to read information from rename queue. */
    typename TimeBuffer<RenameStruct>::wire fromRename;

  public:
    /** ROB interface. */
    ROB *rob;

  private:
    /** Pointer to O3CPU. */
    O3CPU *cpu;

    /** Vector of all of the threads. */
    std::vector<Thread *> thread;

    /** Records that commit has written to the time buffer this cycle. Used for
     * the CPU to determine if it can deschedule itself if there is no activity.
     */
    bool wroteToTimeBuffer;

    /** Records if the number of ROB entries has changed this cycle. If it has,
     * then the number of free entries must be re-broadcast.
     */
    bool changedROBNumEntries[Impl::MaxThreads];

    /** A counter of how many threads are currently squashing. */
    int squashCounter;

    /** Records if a thread has to squash this cycle due to a trap. */
    bool trapSquash[Impl::MaxThreads];

    /** Records if a thread has to squash this cycle due to an XC write. */
    bool tcSquash[Impl::MaxThreads];

    /** Priority List used for Commit Policy */
    std::list<unsigned> priority_list;

    /** IEW to Commit delay, in ticks. */
    unsigned iewToCommitDelay;

    /** Commit to IEW delay, in ticks. */
    unsigned commitToIEWDelay;

    /** Rename to ROB delay, in ticks. */
    unsigned renameToROBDelay;

    unsigned fetchToCommitDelay;

    /** Rename width, in instructions.  Used so ROB knows how many
     *  instructions to get from the rename instruction queue.
     */
    unsigned renameWidth;

    /** Commit width, in instructions. */
    unsigned commitWidth;

    /** Number of Reorder Buffers */
    unsigned numRobs;

    /** Number of Active Threads */
    unsigned numThreads;

    /** Is a drain pending. */
    bool drainPending;

    /** Is commit switched out. */
    bool switchedOut;

    /** The latency to handle a trap.  Used when scheduling trap
     * squash event.
     */
    Tick trapLatency;

    /** The interrupt fault. */
    Fault interrupt;

    /** The commit PC of each thread.  Refers to the instruction that
     * is currently being processed/committed.
     */
    Addr PC[Impl::MaxThreads];

    /** The commit micro PC of each thread.  Refers to the instruction that
     * is currently being processed/committed.
     */
    Addr microPC[Impl::MaxThreads];

    /** The next PC of each thread. */
    Addr nextPC[Impl::MaxThreads];

    /** The next NPC of each thread. */
    Addr nextNPC[Impl::MaxThreads];

    /** The next micro PC of each thread. */
    Addr nextMicroPC[Impl::MaxThreads];

    /** The sequence number of the youngest valid instruction in the ROB. */
    InstSeqNum youngestSeqNum[Impl::MaxThreads];

    /** Records if there is a trap currently in flight. */
    bool trapInFlight[Impl::MaxThreads];

    /** Records if there were any stores committed this cycle. */
    bool committedStores[Impl::MaxThreads];

    /** Records if commit should check if the ROB is truly empty (see
        commit_impl.hh). */
    bool checkEmptyROB[Impl::MaxThreads];

    /** Pointer to the list of active threads. */
    std::list<unsigned> *activeThreads;

    /** Rename map interface. */
    RenameMap *renameMap[Impl::MaxThreads];

    /** Updates commit stats based on this instruction. */
    void updateComInstStats(DynInstPtr &inst);

    /** Stat for the total number of committed instructions. */
    Stats::Scalar<> commitCommittedInsts;
    /** Stat for the total number of squashed instructions discarded by commit.
     */
    Stats::Scalar<> commitSquashedInsts;
    /** Stat for the total number of times commit is told to squash.
     * @todo: Actually increment this stat.
     */
    Stats::Scalar<> commitSquashEvents;
    /** Stat for the total number of times commit has had to stall due to a non-
     * speculative instruction reaching the head of the ROB.
     */
    Stats::Scalar<> commitNonSpecStalls;
    /** Stat for the total number of branch mispredicts that caused a squash. */
    Stats::Scalar<> branchMispredicts;
    /** Distribution of the number of committed instructions each cycle. */
    Stats::Distribution<> numCommittedDist;

    /** Total number of instructions committed. */
    Stats::Vector<> statComInst;
    /** Total number of software prefetches committed. */
    Stats::Vector<> statComSwp;
    /** Stat for the total number of committed memory references. */
    Stats::Vector<> statComRefs;
    /** Stat for the total number of committed loads. */
    Stats::Vector<> statComLoads;
    /** Total number of committed memory barriers. */
    Stats::Vector<> statComMembars;
    /** Total number of committed branches. */
    Stats::Vector<> statComBranches;

    /** Number of cycles where the commit bandwidth limit is reached. */
    Stats::Scalar<> commitEligibleSamples;
    /** Number of instructions not committed due to bandwidth limits. */
    Stats::Vector<> commitEligible;
};

#endif // __CPU_O3_COMMIT_HH__