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

#include "base/intmath.hh"
#include "base/misc.hh"
#include "base/trace.hh"
#include "cpu/o3/2bit_local_pred.hh"

LocalBP::LocalBP(unsigned _localPredictorSize,
                 unsigned _localCtrBits,
                 unsigned _instShiftAmt)
    : localPredictorSize(_localPredictorSize),
      localCtrBits(_localCtrBits),
      instShiftAmt(_instShiftAmt)
{
    if (!isPowerOf2(localPredictorSize)) {
        fatal("Invalid local predictor size!\n");
    }

    localPredictorSets = localPredictorSize / localCtrBits;

    if (!isPowerOf2(localPredictorSets)) {
        fatal("Invalid number of local predictor sets! Check localCtrBits.\n");
    }

    // Setup the index mask.
    indexMask = localPredictorSets - 1;

    DPRINTF(Fetch, "Branch predictor: index mask: %#x\n", indexMask);

    // Setup the array of counters for the local predictor.
    localCtrs.resize(localPredictorSets);

    for (int i = 0; i < localPredictorSets; ++i)
        localCtrs[i].setBits(_localCtrBits);

    DPRINTF(Fetch, "Branch predictor: local predictor size: %i\n",
            localPredictorSize);

    DPRINTF(Fetch, "Branch predictor: local counter bits: %i\n", localCtrBits);

    DPRINTF(Fetch, "Branch predictor: instruction shift amount: %i\n",
            instShiftAmt);
}

void
LocalBP::reset()
{
    for (int i = 0; i < localPredictorSets; ++i) {
        localCtrs[i].reset();
    }
}

bool
LocalBP::lookup(Addr &branch_addr, void * &bp_history)
{
    bool taken;
    uint8_t counter_val;
    unsigned local_predictor_idx = getLocalIndex(branch_addr);

    DPRINTF(Fetch, "Branch predictor: Looking up index %#x\n",
            local_predictor_idx);

    counter_val = localCtrs[local_predictor_idx].read();

    DPRINTF(Fetch, "Branch predictor: prediction is %i.\n",
            (int)counter_val);

    taken = getPrediction(counter_val);

#if 0
    // Speculative update.
    if (taken) {
        DPRINTF(Fetch, "Branch predictor: Branch updated as taken.\n");
        localCtrs[local_predictor_idx].increment();
    } else {
        DPRINTF(Fetch, "Branch predictor: Branch updated as not taken.\n");
        localCtrs[local_predictor_idx].decrement();
    }
#endif

    return taken;
}

void
LocalBP::update(Addr &branch_addr, bool taken, void *bp_history)
{
    assert(bp_history == NULL);
    unsigned local_predictor_idx;

    // Update the local predictor.
    local_predictor_idx = getLocalIndex(branch_addr);

    DPRINTF(Fetch, "Branch predictor: Looking up index %#x\n",
            local_predictor_idx);

    if (taken) {
        DPRINTF(Fetch, "Branch predictor: Branch updated as taken.\n");
        localCtrs[local_predictor_idx].increment();
    } else {
        DPRINTF(Fetch, "Branch predictor: Branch updated as not taken.\n");
        localCtrs[local_predictor_idx].decrement();
    }
}

inline
bool
LocalBP::getPrediction(uint8_t &count)
{
    // Get the MSB of the count
    return (count >> (localCtrBits - 1));
}

inline
unsigned
LocalBP::getLocalIndex(Addr &branch_addr)
{
    return (branch_addr >> instShiftAmt) & indexMask;
}
