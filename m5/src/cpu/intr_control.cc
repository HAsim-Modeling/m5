/*
 * Copyright (c) 2002, 2003, 2004, 2005
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
 * Authors: Nathan L. Binkert
 *          Ronald G. Dreslinski Jr
 */

#include <string>
#include <vector>

#include "base/trace.hh"
#include "cpu/base.hh"
#include "cpu/thread_context.hh"
#include "cpu/intr_control.hh"
#include "sim/sim_object.hh"

using namespace std;

IntrControl::IntrControl(const Params *p)
    : SimObject(p), sys(p->sys)
{}

void
IntrControl::post(int cpu_id, int int_num, int index)
{
    DPRINTF(IntrControl, "post  %d:%d (cpu %d)\n", int_num, index, cpu_id);
    std::vector<ThreadContext *> &tcvec = sys->threadContexts;
    BaseCPU *cpu = tcvec[cpu_id]->getCpuPtr();
    cpu->post_interrupt(int_num, index);
}

void
IntrControl::clear(int cpu_id, int int_num, int index)
{
    DPRINTF(IntrControl, "clear %d:%d (cpu %d)\n", int_num, index, cpu_id);
    std::vector<ThreadContext *> &tcvec = sys->threadContexts;
    BaseCPU *cpu = tcvec[cpu_id]->getCpuPtr();
    cpu->clear_interrupt(int_num, index);
}

IntrControl *
IntrControlParams::create()
{
    return new IntrControl(this);
}
