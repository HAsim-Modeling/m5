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

#include "arch/alpha/tru64/system.hh"
#include "arch/isa_traits.hh"
#include "arch/vtophys.hh"
#include "base/loader/symtab.hh"
#include "base/trace.hh"
#include "cpu/base.hh"
#include "cpu/thread_context.hh"
#include "kern/tru64/tru64_events.hh"
#include "kern/system_events.hh"
#include "mem/physical.hh"
#include "mem/port.hh"

using namespace std;

Tru64AlphaSystem::Tru64AlphaSystem(Tru64AlphaSystem::Params *p)
    : AlphaSystem(p)
{
    Addr addr = 0;
    if (kernelSymtab->findAddress("enable_async_printf", addr)) {
        virtPort.write(addr, (uint32_t)0);
    }

#ifdef DEBUG
    kernelPanicEvent = addKernelFuncEvent<BreakPCEvent>("panic");
    if (!kernelPanicEvent)
        panic("could not find kernel symbol \'panic\'");
#endif

    badaddrEvent = addKernelFuncEvent<BadAddrEvent>("badaddr");
    if (!badaddrEvent)
        panic("could not find kernel symbol \'badaddr\'");

    skipPowerStateEvent =
        addKernelFuncEvent<SkipFuncEvent>("tl_v48_capture_power_state");
    skipScavengeBootEvent =
        addKernelFuncEvent<SkipFuncEvent>("pmap_scavenge_boot");

#if TRACING_ON
    printfEvent = addKernelFuncEvent<PrintfEvent>("printf");
    debugPrintfEvent = addKernelFuncEvent<DebugPrintfEvent>("m5printf");
    debugPrintfrEvent = addKernelFuncEvent<DebugPrintfrEvent>("m5printfr");
    dumpMbufEvent = addKernelFuncEvent<DumpMbufEvent>("m5_dump_mbuf");
#endif
}

Tru64AlphaSystem::~Tru64AlphaSystem()
{
#ifdef DEBUG
    delete kernelPanicEvent;
#endif
    delete badaddrEvent;
    delete skipPowerStateEvent;
    delete skipScavengeBootEvent;
#if TRACING_ON
    delete printfEvent;
    delete debugPrintfEvent;
    delete debugPrintfrEvent;
    delete dumpMbufEvent;
#endif
}

Tru64AlphaSystem *
Tru64AlphaSystemParams::create()
{
    return new Tru64AlphaSystem(this);
}
