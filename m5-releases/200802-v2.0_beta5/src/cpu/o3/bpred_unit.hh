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
 * Authors: Kevin T. Lim
 */

#ifndef __CPU_O3_BPRED_UNIT_HH__
#define __CPU_O3_BPRED_UNIT_HH__

#include "base/statistics.hh"
#include "cpu/inst_seq.hh"

#include "cpu/o3/2bit_local_pred.hh"
#include "cpu/o3/btb.hh"
#include "cpu/o3/ras.hh"
#include "cpu/o3/tournament_pred.hh"

#include "sim/host.hh"

#include <list>

/**
 * Basically a wrapper class to hold both the branch predictor
 * and the BTB.
 */
template<class Impl>
class BPredUnit
{
  private:
    typedef typename Impl::Params Params;
    typedef typename Impl::DynInstPtr DynInstPtr;

    enum PredType {
        Local,
        Tournament
    };

    PredType predictor;

  public:

    /**
     * @param params The params object, that has the size of the BP and BTB.
     */
    BPredUnit(Params *params);

    /**
     * Registers statistics.
     */
    void regStats();

    void switchOut();

    void takeOverFrom();

    /**
     * Predicts whether or not the instruction is a taken branch, and the
     * target of the branch if it is taken.
     * @param inst The branch instruction.
     * @param PC The predicted PC is passed back through this parameter.
     * @param tid The thread id.
     * @return Returns if the branch is taken or not.
     */
    bool predict(DynInstPtr &inst, Addr &PC, unsigned tid);

    // @todo: Rename this function.
    void BPUncond(void * &bp_history);

    /**
     * Tells the branch predictor to commit any updates until the given
     * sequence number.
     * @param done_sn The sequence number to commit any older updates up until.
     * @param tid The thread id.
     */
    void update(const InstSeqNum &done_sn, unsigned tid);

    /**
     * Squashes all outstanding updates until a given sequence number.
     * @param squashed_sn The sequence number to squash any younger updates up
     * until.
     * @param tid The thread id.
     */
    void squash(const InstSeqNum &squashed_sn, unsigned tid);

    /**
     * Squashes all outstanding updates until a given sequence number, and
     * corrects that sn's update with the proper address and taken/not taken.
     * @param squashed_sn The sequence number to squash any younger updates up
     * until.
     * @param corr_target The correct branch target.
     * @param actually_taken The correct branch direction.
     * @param tid The thread id.
     */
    void squash(const InstSeqNum &squashed_sn, const Addr &corr_target,
                bool actually_taken, unsigned tid);

    /**
     * @param bp_history Pointer to the history object.  The predictor
     * will need to update any state and delete the object.
     */
    void BPSquash(void *bp_history);

    /**
     * Looks up a given PC in the BP to see if it is taken or not taken.
     * @param inst_PC The PC to look up.
     * @param bp_history Pointer that will be set to an object that
     * has the branch predictor state associated with the lookup.
     * @return Whether the branch is taken or not taken.
     */
    bool BPLookup(Addr &inst_PC, void * &bp_history);

    /**
     * Looks up a given PC in the BTB to see if a matching entry exists.
     * @param inst_PC The PC to look up.
     * @return Whether the BTB contains the given PC.
     */
    bool BTBValid(Addr &inst_PC)
    { return BTB.valid(inst_PC, 0); }

    /**
     * Looks up a given PC in the BTB to get the predicted target.
     * @param inst_PC The PC to look up.
     * @return The address of the target of the branch.
     */
    Addr BTBLookup(Addr &inst_PC)
    { return BTB.lookup(inst_PC, 0); }

    /**
     * Updates the BP with taken/not taken information.
     * @param inst_PC The branch's PC that will be updated.
     * @param taken Whether the branch was taken or not taken.
     * @param bp_history Pointer to the branch predictor state that is
     * associated with the branch lookup that is being updated.
     * @todo Make this update flexible enough to handle a global predictor.
     */
    void BPUpdate(Addr &inst_PC, bool taken, void *bp_history);

    /**
     * Updates the BTB with the target of a branch.
     * @param inst_PC The branch's PC that will be updated.
     * @param target_PC The branch's target that will be added to the BTB.
     */
    void BTBUpdate(Addr &inst_PC, Addr &target_PC)
    { BTB.update(inst_PC, target_PC,0); }

    void dump();

  private:
    struct PredictorHistory {
        /**
         * Makes a predictor history struct that contains any
         * information needed to update the predictor, BTB, and RAS.
         */
        PredictorHistory(const InstSeqNum &seq_num, const Addr &inst_PC,
                         const bool pred_taken, void *bp_history,
                         const unsigned _tid)
            : seqNum(seq_num), PC(inst_PC), RASTarget(0),
              RASIndex(0), tid(_tid), predTaken(pred_taken), usedRAS(0),
              wasCall(0), bpHistory(bp_history)
        { }

        /** The sequence number for the predictor history entry. */
        InstSeqNum seqNum;

        /** The PC associated with the sequence number. */
        Addr PC;

        /** The RAS target (only valid if a return). */
        Addr RASTarget;

        /** The RAS index of the instruction (only valid if a call). */
        unsigned RASIndex;

        /** The thread id. */
        unsigned tid;

        /** Whether or not it was predicted taken. */
        bool predTaken;

        /** Whether or not the RAS was used. */
        bool usedRAS;

        /** Whether or not the instruction was a call. */
        bool wasCall;

        /** Pointer to the history object passed back from the branch
         * predictor.  It is used to update or restore state of the
         * branch predictor.
         */
        void *bpHistory;
    };

    typedef std::list<PredictorHistory> History;

    /**
     * The per-thread predictor history. This is used to update the predictor
     * as instructions are committed, or restore it to the proper state after
     * a squash.
     */
    History predHist[Impl::MaxThreads];

    /** The local branch predictor. */
    LocalBP *localBP;

    /** The tournament branch predictor. */
    TournamentBP *tournamentBP;

    /** The BTB. */
    DefaultBTB BTB;

    /** The per-thread return address stack. */
    ReturnAddrStack RAS[Impl::MaxThreads];

    /** Stat for number of BP lookups. */
    Stats::Scalar<> lookups;
    /** Stat for number of conditional branches predicted. */
    Stats::Scalar<> condPredicted;
    /** Stat for number of conditional branches predicted incorrectly. */
    Stats::Scalar<> condIncorrect;
    /** Stat for number of BTB lookups. */
    Stats::Scalar<> BTBLookups;
    /** Stat for number of BTB hits. */
    Stats::Scalar<> BTBHits;
    /** Stat for number of times the BTB is correct. */
    Stats::Scalar<> BTBCorrect;
    /** Stat for number of times the RAS is used to get a target. */
    Stats::Scalar<> usedRAS;
    /** Stat for number of times the RAS is incorrect. */
    Stats::Scalar<> RASIncorrect;
};

#endif // __CPU_O3_BPRED_UNIT_HH__
