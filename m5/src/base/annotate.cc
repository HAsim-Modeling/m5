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
 * Authors: Ali G. Sadii
 */

#include "base/annotate.hh"
#include "base/callback.hh"
#include "base/output.hh"
#include "base/trace.hh"
#include "sim/core.hh"
#include "sim/sim_exit.hh"
#include "sim/system.hh"



class AnnotateDumpCallback : public Callback
{
  public:
    virtual void process();
};

void
AnnotateDumpCallback::process()
{
    Annotate::annotations.dump();
}

namespace Annotate {


Annotate annotations;

Annotate::Annotate()
{
    registerExitCallback(new AnnotateDumpCallback);
}

void
Annotate::add(System *sys, Addr stack, uint32_t sm, uint32_t st,
        uint32_t wm, uint32_t ws)
{
    AnnotateData *an;

    an = new AnnotateData;
    an->time = curTick;

    std::map<System*, std::string>::iterator i = nameCache.find(sys);
    if (i == nameCache.end()) {
        nameCache[sys] = sys->name();
    }

    an->system = nameCache[sys];
    an->stack = stack;
    an->stateMachine = sm;
    an->curState = st;
    an->waitMachine = wm;
    an->waitState = ws;

    data.push_back(an);
    if (an->waitMachine)
        DPRINTF(Annotate, "Annotating: %s(%#llX) %d:%d waiting on %d:%d\n",
                an->system, an->stack, an->stateMachine, an->curState,
                an->waitMachine, an->waitState);
    else
        DPRINTF(Annotate, "Annotating: %s(%#llX) %d:%d beginning\n", an->system,
                an->stack, an->stateMachine, an->curState);

    DPRINTF(Annotate, "Now %d events on list\n", data.size());

}

void
Annotate::dump()
{

    std::list<AnnotateData*>::iterator i;

    i = data.begin();

    if (i == data.end())
        return;

    std::ostream *os = simout.create("annotate.dat");

    AnnotateData *an;

    while (i != data.end()) {
        DPRINTF(Annotate, "Writing\n", data.size());
        an = *i;
        ccprintf(*os, "%d %s(%#llX) %d %d %d %d\n", an->time, an->system,
                an->stack, an->stateMachine, an->curState, an->waitMachine,
                an->waitState);
        i++;
    }
}

} //namespace Annotate
