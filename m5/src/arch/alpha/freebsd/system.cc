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
 * Authors: Benjamin S. Nash
 */

/**
 * @file
 * Modifications for the FreeBSD kernel.
 * Based on kern/linux/linux_system.cc.
 *
 */

#include "arch/alpha/freebsd/system.hh"
#include "arch/alpha/system.hh"
#include "arch/isa_traits.hh"
#include "arch/vtophys.hh"
#include "base/loader/symtab.hh"
#include "cpu/thread_context.hh"
#include "mem/physical.hh"
#include "mem/port.hh"
#include "sim/byteswap.hh"

#define TIMER_FREQUENCY 1193180

using namespace std;
using namespace AlphaISA;

FreebsdAlphaSystem::FreebsdAlphaSystem(Params *p)
    : AlphaSystem(p)
{
    /**
     * Any time DELAY is called just skip the function.
     * Shouldn't we actually emulate the delay?
     */
    skipDelayEvent = addKernelFuncEvent<SkipFuncEvent>("DELAY");
    skipCalibrateClocks =
        addKernelFuncEvent<SkipCalibrateClocksEvent>("calibrate_clocks");
}


FreebsdAlphaSystem::~FreebsdAlphaSystem()
{
    delete skipDelayEvent;
    delete skipCalibrateClocks;
}


void
FreebsdAlphaSystem::doCalibrateClocks(ThreadContext *tc)
{
    Addr ppc_vaddr = 0;
    Addr timer_vaddr = 0;

    assert(NumArgumentRegs >= 3);
    ppc_vaddr = (Addr)tc->readIntReg(ArgumentReg[1]);
    timer_vaddr = (Addr)tc->readIntReg(ArgumentReg[2]);

    virtPort.write(ppc_vaddr, (uint32_t)Clock::Frequency);
    virtPort.write(timer_vaddr, (uint32_t)TIMER_FREQUENCY);
}


void
FreebsdAlphaSystem::SkipCalibrateClocksEvent::process(ThreadContext *tc)
{
    SkipFuncEvent::process(tc);
    ((FreebsdAlphaSystem *)tc->getSystemPtr())->doCalibrateClocks(tc);
}

FreebsdAlphaSystem *
FreebsdAlphaSystemParams::create()
{
    return new FreebsdAlphaSystem(this);
}
