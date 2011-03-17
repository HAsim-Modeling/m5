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
 * Authors: Steven K. Reinhardt
 *          Lisa R. Hsu
 *          Nathan L. Binkert
 */

#ifndef __SYSTEM_HH__
#define __SYSTEM_HH__

#include <string>
#include <vector>

#include "base/loader/symtab.hh"
#include "base/misc.hh"
#include "base/statistics.hh"
#include "config/full_system.hh"
#include "cpu/pc_event.hh"
#include "enums/MemoryMode.hh"
#include "mem/port.hh"
#include "params/System.hh"
#include "sim/sim_object.hh"
#if FULL_SYSTEM
#include "kern/system_events.hh"
#include "mem/vport.hh"
#endif

class BaseCPU;
class ThreadContext;
class ObjectFile;
class PhysicalMemory;

#if FULL_SYSTEM
class Platform;
#endif
class GDBListener;
namespace TheISA
{
    class RemoteGDB;
}

class System : public SimObject
{
  public:

    static const char *MemoryModeStrings[3];

    Enums::MemoryMode
    getMemoryMode()
    {
        assert(memoryMode);
        return memoryMode;
    }

    /** Change the memory mode of the system. This should only be called by the
     * python!!
     * @param mode Mode to change to (atomic/timing)
     */
    void setMemoryMode(Enums::MemoryMode mode);

    PhysicalMemory *physmem;
    PCEventQueue pcEventQueue;

    std::vector<ThreadContext *> threadContexts;
    int numcpus;

    int getNumCPUs()
    {
        if (numcpus != threadContexts.size())
            panic("cpu array not fully populated!");

        return numcpus;
    }

#if FULL_SYSTEM
    Platform *platform;
    uint64_t init_param;

    /** Port to physical memory used for writing object files into ram at
     * boot.*/
    FunctionalPort functionalPort;
    VirtualPort virtPort;

    /** kernel symbol table */
    SymbolTable *kernelSymtab;

    /** Object pointer for the kernel code */
    ObjectFile *kernel;

    /** Begining of kernel code */
    Addr kernelStart;

    /** End of kernel code */
    Addr kernelEnd;

    /** Entry point in the kernel to start at */
    Addr kernelEntry;

#else

    uint64_t page_ptr;

  protected:
    uint64_t next_PID;

  public:
    uint64_t allocatePID()
    {
        return next_PID++;
    }


#endif // FULL_SYSTEM

  protected:
    Enums::MemoryMode memoryMode;

#if FULL_SYSTEM
    /**
     * Fix up an address used to match PCs for hooking simulator
     * events on to target function executions.  See comment in
     * system.cc for details.
     */
    virtual Addr fixFuncEventAddr(Addr addr) = 0;

    /**
     * Add a function-based event to the given function, to be looked
     * up in the specified symbol table.
     */
    template <class T>
    T *addFuncEvent(SymbolTable *symtab, const char *lbl)
    {
        Addr addr = 0; // initialize only to avoid compiler warning

        if (symtab->findAddress(lbl, addr)) {
            T *ev = new T(&pcEventQueue, lbl, fixFuncEventAddr(addr));
            return ev;
        }

        return NULL;
    }

    /** Add a function-based event to kernel code. */
    template <class T>
    T *addKernelFuncEvent(const char *lbl)
    {
        return addFuncEvent<T>(kernelSymtab, lbl);
    }

#endif
  public:
    std::vector<TheISA::RemoteGDB *> remoteGDB;
    std::vector<GDBListener *> gdbListen;
    bool breakpoint();

  public:
    typedef SystemParams Params;

  protected:
    Params *_params;

  public:
    System(Params *p);
    ~System();

    void startup();

    const Params *params() const { return (const Params *)_params; }

  public:

#if FULL_SYSTEM
    /**
     * Returns the addess the kernel starts at.
     * @return address the kernel starts at
     */
    Addr getKernelStart() const { return kernelStart; }

    /**
     * Returns the addess the kernel ends at.
     * @return address the kernel ends at
     */
    Addr getKernelEnd() const { return kernelEnd; }

    /**
     * Returns the addess the entry point to the kernel code.
     * @return entry point of the kernel code
     */
    Addr getKernelEntry() const { return kernelEntry; }

#else

    Addr new_page();

#endif // FULL_SYSTEM

    int registerThreadContext(ThreadContext *tc, int tcIndex);
    void replaceThreadContext(ThreadContext *tc, int tcIndex);

    void serialize(std::ostream &os);
    void unserialize(Checkpoint *cp, const std::string &section);

  public:
    ////////////////////////////////////////////
    //
    // STATIC GLOBAL SYSTEM LIST
    //
    ////////////////////////////////////////////

    static std::vector<System *> systemList;
    static int numSystemsRunning;

    static void printSystems();


};

#endif // __SYSTEM_HH__
