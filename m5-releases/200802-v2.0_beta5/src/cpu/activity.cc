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

#include <cstring>

#include "base/timebuf.hh"
#include "cpu/activity.hh"

ActivityRecorder::ActivityRecorder(int num_stages, int longest_latency,
                                   int activity)
    : activityBuffer(longest_latency, 0), longestLatency(longest_latency),
      activityCount(activity), numStages(num_stages)
{
    stageActive = new bool[numStages];
    std::memset(stageActive, 0, numStages);
}

void
ActivityRecorder::activity()
{
    // If we've already recorded activity for this cycle, we don't
    // want to increment the count any more.
    if (activityBuffer[0]) {
        return;
    }

    activityBuffer[0] = true;

    ++activityCount;

    DPRINTF(Activity, "Activity: %i\n", activityCount);
}

void
ActivityRecorder::advance()
{
    // If there's a 1 in the slot that is about to be erased once the
    // time buffer advances, then decrement the activityCount.
    if (activityBuffer[-longestLatency]) {
        --activityCount;

        assert(activityCount >= 0);

        DPRINTF(Activity, "Activity: %i\n", activityCount);

        if (activityCount == 0) {
            DPRINTF(Activity, "No activity left!\n");
        }
    }

    activityBuffer.advance();
}

void
ActivityRecorder::activateStage(const int idx)
{
    // Increment the activity count if this stage wasn't already active.
    if (!stageActive[idx]) {
        ++activityCount;

        stageActive[idx] = true;

        DPRINTF(Activity, "Activity: %i\n", activityCount);
    } else {
        DPRINTF(Activity, "Stage %i already active.\n", idx);
    }

//    assert(activityCount < longestLatency + numStages + 1);
}

void
ActivityRecorder::deactivateStage(const int idx)
{
    // Decrement the activity count if this stage was active.
    if (stageActive[idx]) {
        --activityCount;

        stageActive[idx] = false;

        DPRINTF(Activity, "Activity: %i\n", activityCount);
    } else {
        DPRINTF(Activity, "Stage %i already inactive.\n", idx);
    }

    assert(activityCount >= 0);
}

void
ActivityRecorder::reset()
{
    activityCount = 0;
    std::memset(stageActive, 0, numStages);
    for (int i = 0; i < longestLatency + 1; ++i)
        activityBuffer.advance();
}

void
ActivityRecorder::dump()
{
    for (int i = 0; i <= longestLatency; ++i) {
        cprintf("[Idx:%i %i] ", i, activityBuffer[-i]);
    }

    cprintf("\n");

    for (int i = 0; i < numStages; ++i) {
        cprintf("[Stage:%i %i]\n", i, stageActive[i]);
    }

    cprintf("\n");

    cprintf("Activity count: %i\n", activityCount);
}

void
ActivityRecorder::validate()
{
    int count = 0;
    for (int i = 0; i <= longestLatency; ++i) {
        if (activityBuffer[-i]) {
            count++;
        }
    }

    for (int i = 0; i < numStages; ++i) {
        if (stageActive[i]) {
            count++;
        }
    }

    assert(count == activityCount);
}
