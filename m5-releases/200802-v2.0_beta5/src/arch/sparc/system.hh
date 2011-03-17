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
 * Authors: Ali G. Saidi
 */

#ifndef __ARCH_SPARC_SYSTEM_HH__
#define __ARCH_SPARC_SYSTEM_HH__

#include <string>
#include <vector>

#include "base/loader/symtab.hh"
#include "cpu/pc_event.hh"
#include "kern/system_events.hh"
#include "params/SparcSystem.hh"
#include "sim/sim_object.hh"
#include "sim/system.hh"

class SparcSystem : public System
{
  public:
    typedef SparcSystemParams Params;
    SparcSystem(Params *p);
    ~SparcSystem();

/**
 * Serialization stuff
 */
  public:
    virtual void serialize(std::ostream &os);
    virtual void unserialize(Checkpoint *cp, const std::string &section);

    /** reset binary symbol table */
    SymbolTable *resetSymtab;

    /** hypervison binary symbol table */
    SymbolTable *hypervisorSymtab;

    /** openboot symbol table */
    SymbolTable *openbootSymtab;

    /** nvram symbol table? */
    SymbolTable *nvramSymtab;

    /** hypervisor desc symbol table? */
    SymbolTable *hypervisorDescSymtab;

    /** partition desc symbol table? */
    SymbolTable *partitionDescSymtab;

    /** Object pointer for the reset binary */
    ObjectFile *reset;

    /** Object pointer for the hypervisor code */
    ObjectFile *hypervisor;

    /** Object pointer for the openboot code */
    ObjectFile *openboot;

    /** Object pointer for the nvram image */
    ObjectFile *nvram;

    /** Object pointer for the hypervisor description image */
    ObjectFile *hypervisor_desc;

    /** Object pointer for the partition description image */
    ObjectFile *partition_desc;

    /** System Tick for syncronized tick across all cpus. */
    Tick sysTick;

    /** functional port to ROM */
    FunctionalPort funcRomPort;

    /** functional port to nvram */
    FunctionalPort funcNvramPort;

    /** functional port to hypervisor description */
    FunctionalPort funcHypDescPort;

    /** functional port to partition description */
    FunctionalPort funcPartDescPort;

  protected:
    const Params *params() const { return (const Params *)_params; }

    /** Add a function-based event to reset binary. */
    template <class T>
    T *addResetFuncEvent(const char *lbl)
    {
        return addFuncEvent<T>(resetSymtab, lbl);
    }

    /** Add a function-based event to the hypervisor. */
    template <class T>
    T *addHypervisorFuncEvent(const char *lbl)
    {
        return addFuncEvent<T>(hypervisorSymtab, lbl);
    }

    /** Add a function-based event to the openboot. */
    template <class T>
    T *addOpenbootFuncEvent(const char *lbl)
    {
        return addFuncEvent<T>(openbootSymtab, lbl);
    }

    virtual Addr fixFuncEventAddr(Addr addr)
    {
        //XXX This may eventually have to do something useful.
        return addr;
    }
};

#endif

