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

#ifndef __ARCH_ALPHA_TRU64_SYSTEM_HH__
#define __ARCH_ALPHA_TRU64_SYSTEM_HH__

#include "arch/alpha/system.hh"
#include "arch/isa_traits.hh"
#include "params/Tru64AlphaSystem.hh"
#include "sim/system.hh"

class ThreadContext;

class BreakPCEvent;
class BadAddrEvent;
class SkipFuncEvent;
class PrintfEvent;
class DebugPrintfEvent;
class DebugPrintfrEvent;
class DumpMbufEvent;
class AlphaArguments;

class Tru64AlphaSystem : public AlphaSystem
{
  private:
#ifdef DEBUG
    /** Event to halt the simulator if the kernel calls panic()  */
    BreakPCEvent *kernelPanicEvent;
#endif

    BadAddrEvent *badaddrEvent;
    SkipFuncEvent *skipPowerStateEvent;
    SkipFuncEvent *skipScavengeBootEvent;
    PrintfEvent *printfEvent;
    DebugPrintfEvent  *debugPrintfEvent;
    DebugPrintfrEvent *debugPrintfrEvent;
    DumpMbufEvent *dumpMbufEvent;

  public:
    typedef Tru64AlphaSystemParams Params;
    Tru64AlphaSystem(Params *p);
    ~Tru64AlphaSystem();

    static void Printf(AlphaArguments args);
    static void DumpMbuf(AlphaArguments args);
};

#endif // __ARCH_ALPHA_TRU64_SYSTEM_HH__
