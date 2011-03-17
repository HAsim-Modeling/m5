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
 * Authors: Gabe M. Black
 */

#include <string>

#include "config/full_system.hh"
#include "config/use_checker.hh"
#include "cpu/base.hh"
#include "cpu/o3/sparc/cpu.hh"
#include "cpu/o3/sparc/impl.hh"
#include "cpu/o3/sparc/params.hh"
#include "cpu/o3/fu_pool.hh"
#include "params/DerivO3CPU.hh"

class DerivO3CPU : public SparcO3CPU<SparcSimpleImpl>
{
  public:
    DerivO3CPU(SparcSimpleParams *p)
        : SparcO3CPU<SparcSimpleImpl>(p)
    { }
};

DerivO3CPU *
DerivO3CPUParams::create()
{
    DerivO3CPU *cpu;

#if FULL_SYSTEM
    // Full-system only supports a single thread for the moment.
    int actual_num_threads = 1;
#else
    // In non-full-system mode, we infer the number of threads from
    // the workload if it's not explicitly specified.
    int actual_num_threads =
        (numThreads >= workload.size()) ? numThreads : workload.size();

    if (workload.size() == 0) {
        fatal("Must specify at least one workload!");
    }
#endif

    SparcSimpleParams *params = new SparcSimpleParams;

    params->clock = clock;
    params->phase = phase;

    params->tracer = tracer;

    params->name = name;
    params->numberOfThreads = actual_num_threads;
    params->cpu_id = cpu_id;
    params->activity = activity;

    params->itb = itb;
    params->dtb = dtb;

    params->system = system;
#if FULL_SYSTEM
    params->profile = profile;

    params->do_quiesce = do_quiesce;
    params->do_checkpoint_insts = do_checkpoint_insts;
    params->do_statistics_insts = do_statistics_insts;
#else
    params->workload = workload;
#endif // FULL_SYSTEM

#if USE_CHECKER
    params->checker = checker;
#endif

    params->max_insts_any_thread = max_insts_any_thread;
    params->max_insts_all_threads = max_insts_all_threads;
    params->max_loads_any_thread = max_loads_any_thread;
    params->max_loads_all_threads = max_loads_all_threads;
    params->progress_interval = progress_interval;

    //
    // Caches
    //
    params->cachePorts = cachePorts;

    params->decodeToFetchDelay = decodeToFetchDelay;
    params->renameToFetchDelay = renameToFetchDelay;
    params->iewToFetchDelay = iewToFetchDelay;
    params->commitToFetchDelay = commitToFetchDelay;
    params->fetchWidth = fetchWidth;

    params->renameToDecodeDelay = renameToDecodeDelay;
    params->iewToDecodeDelay = iewToDecodeDelay;
    params->commitToDecodeDelay = commitToDecodeDelay;
    params->fetchToDecodeDelay = fetchToDecodeDelay;
    params->decodeWidth = decodeWidth;

    params->iewToRenameDelay = iewToRenameDelay;
    params->commitToRenameDelay = commitToRenameDelay;
    params->decodeToRenameDelay = decodeToRenameDelay;
    params->renameWidth = renameWidth;

    params->commitToIEWDelay = commitToIEWDelay;
    params->renameToIEWDelay = renameToIEWDelay;
    params->issueToExecuteDelay = issueToExecuteDelay;
    params->dispatchWidth = dispatchWidth;
    params->issueWidth = issueWidth;
    params->wbWidth = wbWidth;
    params->wbDepth = wbDepth;
    params->fuPool = fuPool;

    params->iewToCommitDelay = iewToCommitDelay;
    params->renameToROBDelay = renameToROBDelay;
    params->commitWidth = commitWidth;
    params->squashWidth = squashWidth;
    params->trapLatency = trapLatency;

    params->backComSize = backComSize;
    params->forwardComSize = forwardComSize;

    params->predType = predType;
    params->localPredictorSize = localPredictorSize;
    params->localCtrBits = localCtrBits;
    params->localHistoryTableSize = localHistoryTableSize;
    params->localHistoryBits = localHistoryBits;
    params->globalPredictorSize = globalPredictorSize;
    params->globalCtrBits = globalCtrBits;
    params->globalHistoryBits = globalHistoryBits;
    params->choicePredictorSize = choicePredictorSize;
    params->choiceCtrBits = choiceCtrBits;

    params->BTBEntries = BTBEntries;
    params->BTBTagSize = BTBTagSize;

    params->RASSize = RASSize;

    params->LQEntries = LQEntries;
    params->SQEntries = SQEntries;

    params->SSITSize = SSITSize;
    params->LFSTSize = LFSTSize;

    params->numPhysIntRegs = numPhysIntRegs;
    params->numPhysFloatRegs = numPhysFloatRegs;
    params->numIQEntries = numIQEntries;
    params->numROBEntries = numROBEntries;

    params->smtNumFetchingThreads = smtNumFetchingThreads;

    // Default smtFetchPolicy to "RoundRobin", if necessary.
    std::string round_robin_policy = "RoundRobin";
    std::string single_thread = "SingleThread";

    if (actual_num_threads > 1 && single_thread.compare(smtFetchPolicy) == 0)
        params->smtFetchPolicy = round_robin_policy;
    else
        params->smtFetchPolicy = smtFetchPolicy;

    params->smtIQPolicy    = smtIQPolicy;
    params->smtLSQPolicy    = smtLSQPolicy;
    params->smtLSQThreshold = smtLSQThreshold;
    params->smtROBPolicy   = smtROBPolicy;
    params->smtROBThreshold = smtROBThreshold;
    params->smtCommitPolicy = smtCommitPolicy;

    params->instShiftAmt = 2;

    params->deferRegistration = defer_registration;

    params->functionTrace = function_trace;
    params->functionTraceStart = function_trace_start;

    cpu = new DerivO3CPU(params);

    return cpu;
}
