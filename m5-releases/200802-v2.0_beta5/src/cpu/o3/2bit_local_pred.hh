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

#ifndef __CPU_O3_2BIT_LOCAL_PRED_HH__
#define __CPU_O3_2BIT_LOCAL_PRED_HH__

#include "cpu/o3/sat_counter.hh"
#include "sim/host.hh"

#include <vector>

/**
 * Implements a local predictor that uses the PC to index into a table of
 * counters.  Note that any time a pointer to the bp_history is given, it
 * should be NULL using this predictor because it does not have any branch
 * predictor state that needs to be recorded or updated; the update can be
 * determined solely by the branch being taken or not taken.
 */
class LocalBP
{
  public:
    /**
     * Default branch predictor constructor.
     * @param localPredictorSize Size of the local predictor.
     * @param localCtrBits Number of bits per counter.
     * @param instShiftAmt Offset amount for instructions to ignore alignment.
     */
    LocalBP(unsigned localPredictorSize, unsigned localCtrBits,
            unsigned instShiftAmt);

    /**
     * Looks up the given address in the branch predictor and returns
     * a true/false value as to whether it is taken.
     * @param branch_addr The address of the branch to look up.
     * @param bp_history Pointer to any bp history state.
     * @return Whether or not the branch is taken.
     */
    bool lookup(Addr &branch_addr, void * &bp_history);

    /**
     * Updates the branch predictor with the actual result of a branch.
     * @param branch_addr The address of the branch to update.
     * @param taken Whether or not the branch was taken.
     */
    void update(Addr &branch_addr, bool taken, void *bp_history);

    void squash(void *bp_history)
    { assert(bp_history == NULL); }

    void reset();

  private:
    /**
     *  Returns the taken/not taken prediction given the value of the
     *  counter.
     *  @param count The value of the counter.
     *  @return The prediction based on the counter value.
     */
    inline bool getPrediction(uint8_t &count);

    /** Calculates the local index based on the PC. */
    inline unsigned getLocalIndex(Addr &PC);

    /** Array of counters that make up the local predictor. */
    std::vector<SatCounter> localCtrs;

    /** Size of the local predictor. */
    unsigned localPredictorSize;

    /** Number of sets. */
    unsigned localPredictorSets;

    /** Number of bits of the local predictor's counters. */
    unsigned localCtrBits;

    /** Number of bits to shift the PC when calculating index. */
    unsigned instShiftAmt;

    /** Mask to get index bits. */
    unsigned indexMask;
};

#endif // __CPU_O3_2BIT_LOCAL_PRED_HH__
