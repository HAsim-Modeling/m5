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
 */

#ifndef __CPU_O3_DECODE_HH__
#define __CPU_O3_DECODE_HH__

#include <queue>

#include "base/statistics.hh"
#include "base/timebuf.hh"

/**
 * DefaultDecode class handles both single threaded and SMT
 * decode. Its width is specified by the parameters; each cycles it
 * tries to decode that many instructions. Because instructions are
 * actually decoded when the StaticInst is created, this stage does
 * not do much other than check any PC-relative branches.
 */
template<class Impl>
class DefaultDecode
{
  private:
    // Typedefs from the Impl.
    typedef typename Impl::O3CPU O3CPU;
    typedef typename Impl::DynInstPtr DynInstPtr;
    typedef typename Impl::Params Params;
    typedef typename Impl::CPUPol CPUPol;

    // Typedefs from the CPU policy.
    typedef typename CPUPol::FetchStruct FetchStruct;
    typedef typename CPUPol::DecodeStruct DecodeStruct;
    typedef typename CPUPol::TimeStruct TimeStruct;

  public:
    /** Overall decode stage status. Used to determine if the CPU can
     * deschedule itself due to a lack of activity.
     */
    enum DecodeStatus {
        Active,
        Inactive
    };

    /** Individual thread status. */
    enum ThreadStatus {
        Running,
        Idle,
        StartSquash,
        Squashing,
        Blocked,
        Unblocking
    };

  private:
    /** Decode status. */
    DecodeStatus _status;

    /** Per-thread status. */
    ThreadStatus decodeStatus[Impl::MaxThreads];

  public:
    /** DefaultDecode constructor. */
    DefaultDecode(O3CPU *_cpu, Params *params);

    /** Returns the name of decode. */
    std::string name() const;

    /** Registers statistics. */
    void regStats();

    /** Sets the main backwards communication time buffer pointer. */
    void setTimeBuffer(TimeBuffer<TimeStruct> *tb_ptr);

    /** Sets pointer to time buffer used to communicate to the next stage. */
    void setDecodeQueue(TimeBuffer<DecodeStruct> *dq_ptr);

    /** Sets pointer to time buffer coming from fetch. */
    void setFetchQueue(TimeBuffer<FetchStruct> *fq_ptr);

    /** Sets pointer to list of active threads. */
    void setActiveThreads(std::list<unsigned> *at_ptr);

    /** Drains the decode stage. */
    bool drain();

    /** Resumes execution after a drain. */
    void resume() { }

    /** Switches out the decode stage. */
    void switchOut() { }

    /** Takes over from another CPU's thread. */
    void takeOverFrom();

    /** Ticks decode, processing all input signals and decoding as many
     * instructions as possible.
     */
    void tick();

    /** Determines what to do based on decode's current status.
     * @param status_change decode() sets this variable if there was a status
     * change (ie switching from from blocking to unblocking).
     * @param tid Thread id to decode instructions from.
     */
    void decode(bool &status_change, unsigned tid);

    /** Processes instructions from fetch and passes them on to rename.
     * Decoding of instructions actually happens when they are created in
     * fetch, so this function mostly checks if PC-relative branches are
     * correct.
     */
    void decodeInsts(unsigned tid);

  private:
    /** Inserts a thread's instructions into the skid buffer, to be decoded
     * once decode unblocks.
     */
    void skidInsert(unsigned tid);

    /** Returns if all of the skid buffers are empty. */
    bool skidsEmpty();

    /** Updates overall decode status based on all of the threads' statuses. */
    void updateStatus();

    /** Separates instructions from fetch into individual lists of instructions
     * sorted by thread.
     */
    void sortInsts();

    /** Reads all stall signals from the backwards communication timebuffer. */
    void readStallSignals(unsigned tid);

    /** Checks all input signals and updates decode's status appropriately. */
    bool checkSignalsAndUpdate(unsigned tid);

    /** Checks all stall signals, and returns if any are true. */
    bool checkStall(unsigned tid) const;

    /** Returns if there any instructions from fetch on this cycle. */
    inline bool fetchInstsValid();

    /** Switches decode to blocking, and signals back that decode has
     * become blocked.
     * @return Returns true if there is a status change.
     */
    bool block(unsigned tid);

    /** Switches decode to unblocking if the skid buffer is empty, and
     * signals back that decode has unblocked.
     * @return Returns true if there is a status change.
     */
    bool unblock(unsigned tid);

    /** Squashes if there is a PC-relative branch that was predicted
     * incorrectly. Sends squash information back to fetch.
     */
    void squash(DynInstPtr &inst, unsigned tid);

  public:
    /** Squashes due to commit signalling a squash. Changes status to
     * squashing and clears block/unblock signals as needed.
     */
    unsigned squash(unsigned tid);

  private:
    // Interfaces to objects outside of decode.
    /** CPU interface. */
    O3CPU *cpu;

    /** Time buffer interface. */
    TimeBuffer<TimeStruct> *timeBuffer;

    /** Wire to get rename's output from backwards time buffer. */
    typename TimeBuffer<TimeStruct>::wire fromRename;

    /** Wire to get iew's information from backwards time buffer. */
    typename TimeBuffer<TimeStruct>::wire fromIEW;

    /** Wire to get commit's information from backwards time buffer. */
    typename TimeBuffer<TimeStruct>::wire fromCommit;

    /** Wire to write information heading to previous stages. */
    // Might not be the best name as not only fetch will read it.
    typename TimeBuffer<TimeStruct>::wire toFetch;

    /** Decode instruction queue. */
    TimeBuffer<DecodeStruct> *decodeQueue;

    /** Wire used to write any information heading to rename. */
    typename TimeBuffer<DecodeStruct>::wire toRename;

    /** Fetch instruction queue interface. */
    TimeBuffer<FetchStruct> *fetchQueue;

    /** Wire to get fetch's output from fetch queue. */
    typename TimeBuffer<FetchStruct>::wire fromFetch;

    /** Queue of all instructions coming from fetch this cycle. */
    std::queue<DynInstPtr> insts[Impl::MaxThreads];

    /** Skid buffer between fetch and decode. */
    std::queue<DynInstPtr> skidBuffer[Impl::MaxThreads];

    /** Variable that tracks if decode has written to the time buffer this
     * cycle. Used to tell CPU if there is activity this cycle.
     */
    bool wroteToTimeBuffer;

    /** Source of possible stalls. */
    struct Stalls {
        bool rename;
        bool iew;
        bool commit;
    };

    /** Tracks which stages are telling decode to stall. */
    Stalls stalls[Impl::MaxThreads];

    /** Rename to decode delay, in ticks. */
    unsigned renameToDecodeDelay;

    /** IEW to decode delay, in ticks. */
    unsigned iewToDecodeDelay;

    /** Commit to decode delay, in ticks. */
    unsigned commitToDecodeDelay;

    /** Fetch to decode delay, in ticks. */
    unsigned fetchToDecodeDelay;

    /** The width of decode, in instructions. */
    unsigned decodeWidth;

    /** Index of instructions being sent to rename. */
    unsigned toRenameIndex;

    /** number of Active Threads*/
    unsigned numThreads;

    /** List of active thread ids */
    std::list<unsigned> *activeThreads;

    /** Number of branches in flight. */
    unsigned branchCount[Impl::MaxThreads];

    /** Maximum size of the skid buffer. */
    unsigned skidBufferMax;

    /** SeqNum of Squashing Branch Delay Instruction (used for MIPS)*/
    Addr bdelayDoneSeqNum[Impl::MaxThreads];

    /** Instruction used for squashing branch (used for MIPS)*/
    DynInstPtr squashInst[Impl::MaxThreads];

    /** Tells when their is a pending delay slot inst. to send
     *  to rename. If there is, then wait squash after the next
     *  instruction (used for MIPS).
     */
    bool squashAfterDelaySlot[Impl::MaxThreads];


    /** Stat for total number of idle cycles. */
    Stats::Scalar<> decodeIdleCycles;
    /** Stat for total number of blocked cycles. */
    Stats::Scalar<> decodeBlockedCycles;
    /** Stat for total number of normal running cycles. */
    Stats::Scalar<> decodeRunCycles;
    /** Stat for total number of unblocking cycles. */
    Stats::Scalar<> decodeUnblockCycles;
    /** Stat for total number of squashing cycles. */
    Stats::Scalar<> decodeSquashCycles;
    /** Stat for number of times a branch is resolved at decode. */
    Stats::Scalar<> decodeBranchResolved;
    /** Stat for number of times a branch mispredict is detected. */
    Stats::Scalar<> decodeBranchMispred;
    /** Stat for number of times decode detected a non-control instruction
     * incorrectly predicted as a branch.
     */
    Stats::Scalar<> decodeControlMispred;
    /** Stat for total number of decoded instructions. */
    Stats::Scalar<> decodeDecodedInsts;
    /** Stat for total number of squashed instructions. */
    Stats::Scalar<> decodeSquashedInsts;
};

#endif // __CPU_O3_DECODE_HH__