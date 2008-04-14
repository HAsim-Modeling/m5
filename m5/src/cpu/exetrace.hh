/*
 * Copyright (c) 2001, 2002, 2003, 2004, 2005
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
 * Authors: Steven K. Reinhardt
 *          Nathan L. Binkert
 */

#ifndef __EXETRACE_HH__
#define __EXETRACE_HH__

#include "base/trace.hh"
#include "cpu/static_inst.hh"
#include "sim/host.hh"
#include "sim/insttracer.hh"
#include "params/ExeTracer.hh"

class ThreadContext;


namespace Trace {

class ExeTracerRecord : public InstRecord
{
  public:
    ExeTracerRecord(Tick _when, ThreadContext *_thread,
               const StaticInstPtr &_staticInst, Addr _pc, bool spec)
        : InstRecord(_when, _thread, _staticInst, _pc, spec)
    {
    }

    void dump();
};

class ExeTracer : public InstTracer
{
  public:
    typedef ExeTracerParams Params;
    ExeTracer(const Params *params) : InstTracer(params)
    {}

    InstRecord *
    getInstRecord(Tick when, ThreadContext *tc,
            const StaticInstPtr staticInst, Addr pc)
    {
        if (!IsOn(ExecEnable))
            return NULL;

        if (!Trace::enabled)
            return NULL;

        if (!IsOn(ExecSpeculative) && tc->misspeculating())
            return NULL;

        return new ExeTracerRecord(when, tc,
                staticInst, pc, tc->misspeculating());
    }
};

/* namespace Trace */ }

#endif // __EXETRACE_HH__
