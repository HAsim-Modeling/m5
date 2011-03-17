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

#ifndef __CPU_O3_PARAMS_HH__
#define __CPU_O3_PARAMS_HH__

#include "cpu/o3/cpu.hh"

//Forward declarations
class FUPool;

/**
 * This file defines the parameters that will be used for the O3CPU.
 * This must be defined externally so that the Impl can have a params class
 * defined that it can pass to all of the individual stages.
 */
class O3Params : public BaseO3CPU::Params
{
  public:
    unsigned activity;

    //
    // Pointers to key objects
    //
#if !FULL_SYSTEM
    std::vector<Process *> workload;
    Process *process;
#endif // FULL_SYSTEM

    BaseCPU *checker;

    //
    // Caches
    //
    //    MemInterface *icacheInterface;
    //    MemInterface *dcacheInterface;

    unsigned cachePorts;

    //
    // Fetch
    //
    unsigned decodeToFetchDelay;
    unsigned renameToFetchDelay;
    unsigned iewToFetchDelay;
    unsigned commitToFetchDelay;
    unsigned fetchWidth;

    //
    // Decode
    //
    unsigned renameToDecodeDelay;
    unsigned iewToDecodeDelay;
    unsigned commitToDecodeDelay;
    unsigned fetchToDecodeDelay;
    unsigned decodeWidth;

    //
    // Rename
    //
    unsigned iewToRenameDelay;
    unsigned commitToRenameDelay;
    unsigned decodeToRenameDelay;
    unsigned renameWidth;

    //
    // IEW
    //
    unsigned commitToIEWDelay;
    unsigned renameToIEWDelay;
    unsigned issueToExecuteDelay;
    unsigned dispatchWidth;
    unsigned issueWidth;
    unsigned wbWidth;
    unsigned wbDepth;
    FUPool *fuPool;

    //
    // Commit
    //
    unsigned iewToCommitDelay;
    unsigned renameToROBDelay;
    unsigned commitWidth;
    unsigned squashWidth;
    Tick trapLatency;
    Tick fetchTrapLatency;

    //
    // Timebuffer sizes
    //
    unsigned backComSize;
    unsigned forwardComSize;

    //
    // Branch predictor (BP, BTB, RAS)
    //
    std::string predType;
    unsigned localPredictorSize;
    unsigned localCtrBits;
    unsigned localHistoryTableSize;
    unsigned localHistoryBits;
    unsigned globalPredictorSize;
    unsigned globalCtrBits;
    unsigned globalHistoryBits;
    unsigned choicePredictorSize;
    unsigned choiceCtrBits;

    unsigned BTBEntries;
    unsigned BTBTagSize;

    unsigned RASSize;

    //
    // Load store queue
    //
    unsigned LQEntries;
    unsigned SQEntries;

    //
    // Memory dependence
    //
    unsigned SSITSize;
    unsigned LFSTSize;

    //
    // Miscellaneous
    //
    unsigned numPhysIntRegs;
    unsigned numPhysFloatRegs;
    unsigned numIQEntries;
    unsigned numROBEntries;

    //SMT Parameters
    unsigned smtNumFetchingThreads;

    std::string   smtFetchPolicy;

    std::string   smtIQPolicy;
    unsigned smtIQThreshold;

    std::string   smtLSQPolicy;
    unsigned smtLSQThreshold;

    std::string   smtCommitPolicy;

    std::string   smtROBPolicy;
    unsigned smtROBThreshold;

    // Probably can get this from somewhere.
    unsigned instShiftAmt;
};

#endif // __CPU_O3_ALPHA_PARAMS_HH__
