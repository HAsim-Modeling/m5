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

#ifndef __CPU_O3_ROB_HH__
#define __CPU_O3_ROB_HH__

#include <string>
#include <utility>
#include <vector>

/**
 * ROB class.  The ROB is largely what drives squashing.
 */
template <class Impl>
class ROB
{
  protected:
    typedef TheISA::RegIndex RegIndex;
  public:
    //Typedefs from the Impl.
    typedef typename Impl::O3CPU O3CPU;
    typedef typename Impl::DynInstPtr DynInstPtr;

    typedef std::pair<RegIndex, PhysRegIndex> UnmapInfo;
    typedef typename std::list<DynInstPtr>::iterator InstIt;

    /** Possible ROB statuses. */
    enum Status {
        Running,
        Idle,
        ROBSquashing
    };

    /** SMT ROB Sharing Policy */
    enum ROBPolicy{
        Dynamic,
        Partitioned,
        Threshold
    };

  private:
    /** Per-thread ROB status. */
    Status robStatus[Impl::MaxThreads];

    /** ROB resource sharing policy for SMT mode. */
    ROBPolicy robPolicy;

  public:
    /** ROB constructor.
     *  @param _numEntries      Number of entries in ROB.
     *  @param _squashWidth     Number of instructions that can be squashed in a
     *                          single cycle.
     *  @param _smtROBPolicy    ROB Partitioning Scheme for SMT.
     *  @param _smtROBThreshold Max Resources(by %) a thread can have in the ROB.
     *  @param _numThreads      The number of active threads.
     */
    ROB(O3CPU *_cpu, unsigned _numEntries, unsigned _squashWidth,
        std::string smtROBPolicy, unsigned _smtROBThreshold,
        unsigned _numThreads);

    std::string name() const;

    /** Sets pointer to the list of active threads.
     *  @param at_ptr Pointer to the list of active threads.
     */
    void setActiveThreads(std::list<unsigned>* at_ptr);

    /** Switches out the ROB. */
    void switchOut();

    /** Takes over another CPU's thread. */
    void takeOverFrom();

    /** Function to insert an instruction into the ROB. Note that whatever
     *  calls this function must ensure that there is enough space within the
     *  ROB for the new instruction.
     *  @param inst The instruction being inserted into the ROB.
     */
    void insertInst(DynInstPtr &inst);

    /** Returns pointer to the head instruction within the ROB.  There is
     *  no guarantee as to the return value if the ROB is empty.
     *  @retval Pointer to the DynInst that is at the head of the ROB.
     */
//    DynInstPtr readHeadInst();

    /** Returns a pointer to the head instruction of a specific thread within
     *  the ROB.
     *  @return Pointer to the DynInst that is at the head of the ROB.
     */
    DynInstPtr readHeadInst(unsigned tid);

    /** Returns pointer to the tail instruction within the ROB.  There is
     *  no guarantee as to the return value if the ROB is empty.
     *  @retval Pointer to the DynInst that is at the tail of the ROB.
     */
//    DynInstPtr readTailInst();

    /** Returns a pointer to the tail instruction of a specific thread within
     *  the ROB.
     *  @return Pointer to the DynInst that is at the tail of the ROB.
     */
    DynInstPtr readTailInst(unsigned tid);

    /** Retires the head instruction, removing it from the ROB. */
//    void retireHead();

    /** Retires the head instruction of a specific thread, removing it from the
     *  ROB.
     */
    void retireHead(unsigned tid);

    /** Is the oldest instruction across all threads ready. */
//    bool isHeadReady();

    /** Is the oldest instruction across a particular thread ready. */
    bool isHeadReady(unsigned tid);

    /** Is there any commitable head instruction across all threads ready. */
    bool canCommit();

    /** Re-adjust ROB partitioning. */
    void resetEntries();

    /** Number of entries needed For 'num_threads' amount of threads. */
    int entryAmount(int num_threads);

    /** Returns the number of total free entries in the ROB. */
    unsigned numFreeEntries();

    /** Returns the number of free entries in a specific ROB paritition. */
    unsigned numFreeEntries(unsigned tid);

    /** Returns the maximum number of entries for a specific thread. */
    unsigned getMaxEntries(unsigned tid)
    { return maxEntries[tid]; }

    /** Returns the number of entries being used by a specific thread. */
    unsigned getThreadEntries(unsigned tid)
    { return threadEntries[tid]; }

    /** Returns if the ROB is full. */
    bool isFull()
    { return numInstsInROB == numEntries; }

    /** Returns if a specific thread's partition is full. */
    bool isFull(unsigned tid)
    { return threadEntries[tid] == numEntries; }

    /** Returns if the ROB is empty. */
    bool isEmpty()
    { return numInstsInROB == 0; }

    /** Returns if a specific thread's partition is empty. */
    bool isEmpty(unsigned tid)
    { return threadEntries[tid] == 0; }

    /** Executes the squash, marking squashed instructions. */
    void doSquash(unsigned tid);

    /** Squashes all instructions younger than the given sequence number for
     *  the specific thread.
     */
    void squash(InstSeqNum squash_num, unsigned tid);

    /** Updates the head instruction with the new oldest instruction. */
    void updateHead();

    /** Updates the tail instruction with the new youngest instruction. */
    void updateTail();

    /** Reads the PC of the oldest head instruction. */
//    uint64_t readHeadPC();

    /** Reads the PC of the head instruction of a specific thread. */
//    uint64_t readHeadPC(unsigned tid);

    /** Reads the next PC of the oldest head instruction. */
//    uint64_t readHeadNextPC();

    /** Reads the next PC of the head instruction of a specific thread. */
//    uint64_t readHeadNextPC(unsigned tid);

    /** Reads the sequence number of the oldest head instruction. */
//    InstSeqNum readHeadSeqNum();

    /** Reads the sequence number of the head instruction of a specific thread.
     */
//    InstSeqNum readHeadSeqNum(unsigned tid);

    /** Reads the PC of the youngest tail instruction. */
//    uint64_t readTailPC();

    /** Reads the PC of the tail instruction of a specific thread. */
//    uint64_t readTailPC(unsigned tid);

    /** Reads the sequence number of the youngest tail instruction. */
//    InstSeqNum readTailSeqNum();

    /** Reads the sequence number of tail instruction of a specific thread. */
//    InstSeqNum readTailSeqNum(unsigned tid);

    /** Checks if the ROB is still in the process of squashing instructions.
     *  @retval Whether or not the ROB is done squashing.
     */
    bool isDoneSquashing(unsigned tid) const
    { return doneSquashing[tid]; }

    /** Checks if the ROB is still in the process of squashing instructions for
     *  any thread.
     */
    bool isDoneSquashing();

    /** This is more of a debugging function than anything.  Use
     *  numInstsInROB to get the instructions in the ROB unless you are
     *  double checking that variable.
     */
    int countInsts();

    /** This is more of a debugging function than anything.  Use
     *  threadEntries to get the instructions in the ROB unless you are
     *  double checking that variable.
     */
    int countInsts(unsigned tid);

  private:
    /** Pointer to the CPU. */
    O3CPU *cpu;

    /** Active Threads in CPU */
    std::list<unsigned>* activeThreads;

    /** Number of instructions in the ROB. */
    unsigned numEntries;

    /** Entries Per Thread */
    unsigned threadEntries[Impl::MaxThreads];

    /** Max Insts a Thread Can Have in the ROB */
    unsigned maxEntries[Impl::MaxThreads];

    /** ROB List of Instructions */
    std::list<DynInstPtr> instList[Impl::MaxThreads];

    /** Number of instructions that can be squashed in a single cycle. */
    unsigned squashWidth;

  public:
    /** Iterator pointing to the instruction which is the last instruction
     *  in the ROB.  This may at times be invalid (ie when the ROB is empty),
     *  however it should never be incorrect.
     */
    InstIt tail;

    /** Iterator pointing to the instruction which is the first instruction in
     *  in the ROB*/
    InstIt head;

  private:
    /** Iterator used for walking through the list of instructions when
     *  squashing.  Used so that there is persistent state between cycles;
     *  when squashing, the instructions are marked as squashed but not
     *  immediately removed, meaning the tail iterator remains the same before
     *  and after a squash.
     *  This will always be set to cpu->instList.end() if it is invalid.
     */
    InstIt squashIt[Impl::MaxThreads];

  public:
    /** Number of instructions in the ROB. */
    int numInstsInROB;

    /** Dummy instruction returned if there are no insts left. */
    DynInstPtr dummyInst;

  private:
    /** The sequence number of the squashed instruction. */
    InstSeqNum squashedSeqNum[Impl::MaxThreads];

    /** Is the ROB done squashing. */
    bool doneSquashing[Impl::MaxThreads];

    /** Number of active threads. */
    unsigned numThreads;
};

#endif //__CPU_O3_ROB_HH__