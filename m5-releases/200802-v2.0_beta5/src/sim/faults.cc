/*
 * Copyright (c) 2003, 2004, 2005
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
 *          Gabe M. Black
 */

#include "arch/isa_traits.hh"
#include "base/misc.hh"
#include "cpu/thread_context.hh"
#include "cpu/base.hh"
#include "sim/faults.hh"
#include "sim/process.hh"
#include "mem/page_table.hh"

#if !FULL_SYSTEM
void FaultBase::invoke(ThreadContext * tc)
{
    fatal("fault (%s) detected @ PC %p", name(), tc->readPC());
}
#else
void FaultBase::invoke(ThreadContext * tc)
{
    DPRINTF(Fault, "Fault %s at PC: %#x\n", name(), tc->readPC());
    tc->getCpuPtr()->recordEvent(csprintf("Fault %s", name()));

    assert(!tc->misspeculating());
}
#endif

void UnimpFault::invoke(ThreadContext * tc)
{
    fatal("Unimpfault: %s\n", panicStr.c_str());
}

#if !FULL_SYSTEM
void GenericPageTableFault::invoke(ThreadContext *tc)
{
    Process *p = tc->getProcessPtr();

    if (!p->checkAndAllocNextPage(vaddr))
        panic("Page table fault when accessing virtual address %#x\n", vaddr);

}

void GenericAlignmentFault::invoke(ThreadContext *tc)
{
    panic("Alignment fault when accessing virtual address %#x\n", vaddr);
}
#endif
