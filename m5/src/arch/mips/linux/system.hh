/*
 * Copyright (c) 2004, 2005, 2006
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
 * Authors: Ali G. Saidi
 *          Lisa R. Hsu
 *          Nathan L. Binkert
 */

#ifndef __ARCH_MIPS_LINUX_SYSTEM_HH__
#define __ARCH_MIPS_LINUX_SYSTEM_HH__

class ThreadContext;

class BreakPCEvent;
class IdleStartEvent;

#include "arch/mips/idle_event.hh"
#include "arch/mips/system.hh"
#include "kern/linux/events.hh"
#include "params/LinuxMipsSystem.hh"

using namespace MipsISA;
using namespace Linux;

/**
 * This class contains linux specific system code (Loading, Events).
 * It points to objects that are the system binaries to load and patches them
 * appropriately to work in simulator.
 */
class LinuxMipsSystem : public MipsSystem
{
  private:
    class SkipDelayLoopEvent : public SkipFuncEvent
    {
      public:
        SkipDelayLoopEvent(PCEventQueue *q, const std::string &desc, Addr addr)
            : SkipFuncEvent(q, desc, addr) {}
        virtual void process(ThreadContext *tc);
    };

    class PrintThreadInfo : public PCEvent
    {
      public:
        PrintThreadInfo(PCEventQueue *q, const std::string &desc, Addr addr)
            : PCEvent(q, desc, addr) {}
        virtual void process(ThreadContext *tc);
    };


    /**
     * Addresses defining where the kernel bootloader places various
     * elements.  Details found in include/asm-mips/system.h
     */
    Addr KernelStart; // Lookup the symbol swapper_pg_dir

  public:
    Addr InitStack() const { return KernelStart + 0x02000; }
    Addr EmptyPGT() const  { return KernelStart + 0x04000; }
    Addr EmptyPGE() const  { return KernelStart + 0x08000; }
    Addr ZeroPGE() const   { return KernelStart + 0x0A000; }
    Addr StartAddr() const { return KernelStart + 0x10000; }

    Addr Param() const { return ZeroPGE() + 0x0; }
    Addr CommandLine() const { return Param() + 0x0; }
    Addr InitrdStart() const { return Param() + 0x100; }
    Addr InitrdSize() const { return Param() + 0x108; }
    static const int CommandLineSize = 256;

  private:
#ifndef NDEBUG
    /** Event to halt the simulator if the kernel calls panic()  */
    BreakPCEvent *kernelPanicEvent;

    /** Event to halt the simulator if the kernel calls die_if_kernel  */
    BreakPCEvent *kernelDieEvent;
#endif

    /**
     * Event to skip determine_cpu_caches() because we don't support
     * the IPRs that the code can access to figure out cache sizes
     */
    SkipFuncEvent *skipCacheProbeEvent;

    /** PC based event to skip the ide_delay_50ms() call */
    SkipFuncEvent *skipIdeDelay50msEvent;

    /**
     * PC based event to skip the dprink() call and emulate its
     * functionality
     */
    DebugPrintkEvent *debugPrintkEvent;

    /**
     * Skip calculate_delay_loop() rather than waiting for this to be
     * calculated
     */
    SkipDelayLoopEvent *skipDelayLoopEvent;

    /**
     * Event to print information about thread switches if the trace flag
     * Thread is set
     */
    PrintThreadInfo *printThreadEvent;

    /** Grab the PCBB of the idle process when it starts */
    IdleStartEvent *idleStartEvent;

  public:
    typedef LinuxMipsSystemParams Params;
    LinuxMipsSystem(Params *p);
    ~LinuxMipsSystem();

    void setDelayLoop(ThreadContext *tc);
};

#endif // __ARCH_MIPS_LINUX_SYSTEM_HH__
