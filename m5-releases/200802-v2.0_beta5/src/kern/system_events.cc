/*
 * Copyright (c) 2004, 2005
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
 * Authors: Lisa R. Hsu
 *          Nathan L. Binkert
 */

//For ISA_HAS_DELAY_SLOT
#include "arch/isa_traits.hh"
#include "base/trace.hh"
#include "cpu/thread_context.hh"
#include "kern/system_events.hh"

using namespace TheISA;

void
SkipFuncEvent::process(ThreadContext *tc)
{
    Addr newpc = tc->readIntReg(ReturnAddressReg);

    DPRINTF(PCEvent, "skipping %s: pc=%x, newpc=%x\n", description,
            tc->readPC(), newpc);

    tc->setPC(newpc);
    tc->setNextPC(tc->readPC() + sizeof(TheISA::MachInst));
#if ISA_HAS_DELAY_SLOT
    tc->setNextPC(tc->readNextPC() + sizeof(TheISA::MachInst));
#endif
}
