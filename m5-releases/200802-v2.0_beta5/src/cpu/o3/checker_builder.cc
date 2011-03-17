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

#include <string>

#include "cpu/checker/cpu_impl.hh"
#include "cpu/inst_seq.hh"
#include "cpu/o3/alpha/dyn_inst.hh"
#include "cpu/o3/alpha/impl.hh"
#include "params/O3Checker.hh"
#include "sim/process.hh"
#include "sim/sim_object.hh"

class MemObject;

template
class Checker<RefCountingPtr<AlphaDynInst<AlphaSimpleImpl> > >;

/**
 * Specific non-templated derived class used for SimObject configuration.
 */
class O3Checker : public Checker<RefCountingPtr<AlphaDynInst<AlphaSimpleImpl> > >
{
  public:
    O3Checker(Params *p)
        : Checker<RefCountingPtr<AlphaDynInst<AlphaSimpleImpl> > >(p)
    { }
};

////////////////////////////////////////////////////////////////////////
//
//  CheckerCPU Simulation Object
//
O3Checker *
O3CheckerParams::create()
{
    O3Checker::Params *params = new O3Checker::Params();
    params->name = name;
    params->numberOfThreads = 1;
    params->max_insts_any_thread = 0;
    params->max_insts_all_threads = 0;
    params->max_loads_any_thread = 0;
    params->max_loads_all_threads = 0;
    params->exitOnError = exitOnError;
    params->updateOnError = updateOnError;
    params->warnOnlyOnLoadError = warnOnlyOnLoadError;
    params->deferRegistration = defer_registration;
    params->functionTrace = function_trace;
    params->functionTraceStart = function_trace_start;
    params->clock = clock;
    // Hack to touch all parameters.  Consider not deriving Checker
    // from BaseCPU..it's not really a CPU in the end.
    Counter temp;
    temp = max_insts_any_thread;
    temp = max_insts_all_threads;
    temp = max_loads_any_thread;
    temp = max_loads_all_threads;
    Tick temp2 = progress_interval;
    params->progress_interval = 0;
    temp2++;

    params->itb = itb;
    params->dtb = dtb;
    params->system = system;
    params->cpu_id = cpu_id;
#if FULL_SYSTEM
    params->profile = profile;
#else
    params->process = workload;
#endif

    O3Checker *cpu = new O3Checker(params);
    return cpu;
}
