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
 *          Lisa R. Hsu
 */

#include "arch/alpha/ev5.hh"
#include "arch/isa_traits.hh"
#include "cpu/thread_context.hh"
#include "cpu/base.hh"
#include "kern/system_events.hh"
#include "kern/tru64/tru64_events.hh"
#include "kern/tru64/dump_mbuf.hh"
#include "kern/tru64/printf.hh"
#include "sim/arguments.hh"
#include "sim/system.hh"

using namespace TheISA;

//void SkipFuncEvent::process(ExecContext *tc);

void
BadAddrEvent::process(ThreadContext *tc)
{
    // The following gross hack is the equivalent function to the
    // annotation for vmunix::badaddr in:
    // simos/simulation/apps/tcl/osf/tlaser.tcl

    uint64_t a0 = tc->readIntReg(ArgumentReg[0]);

    AddrRangeList resp;
    bool snoop;
    AddrRangeIter iter;
    bool found = false;

    tc->getPhysPort()->getPeerAddressRanges(resp, snoop);
    for(iter = resp.begin(); iter != resp.end(); iter++)
    {
        if (*iter == (TheISA::K0Seg2Phys(a0) & EV5::PAddrImplMask))
            found = true;
    }

    if (!TheISA::IsK0Seg(a0) || found ) {

        DPRINTF(BADADDR, "badaddr arg=%#x bad\n", a0);
        tc->setIntReg(ReturnValueReg, 0x1);
        SkipFuncEvent::process(tc);
    }
    else
        DPRINTF(BADADDR, "badaddr arg=%#x good\n", a0);
}

void
PrintfEvent::process(ThreadContext *tc)
{
    if (DTRACE(Printf)) {
        StringWrap name(tc->getSystemPtr()->name());
        DPRINTFN("");

        Arguments args(tc);
        tru64::Printf(args);
    }
}

void
DebugPrintfEvent::process(ThreadContext *tc)
{
    if (DTRACE(DebugPrintf)) {
        if (!raw) {
            StringWrap name(tc->getSystemPtr()->name());
            DPRINTFN("");
        }

        Arguments args(tc);
        tru64::Printf(args);
    }
}

void
DumpMbufEvent::process(ThreadContext *tc)
{
    if (DTRACE(DebugPrintf)) {
        Arguments args(tc);
        tru64::DumpMbuf(args);
    }
}
