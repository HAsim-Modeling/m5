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
 *          Nathan L. Binkert
 */

#ifndef __ARCH_ALPHA_SYSTEM_HH__
#define __ARCH_ALPHA_SYSTEM_HH__

#include <string>
#include <vector>

#include "base/loader/symtab.hh"
#include "cpu/pc_event.hh"
#include "kern/system_events.hh"
#include "params/AlphaSystem.hh"
#include "sim/sim_object.hh"
#include "sim/system.hh"

class AlphaSystem : public System
{
  public:
    typedef AlphaSystemParams Params;
    AlphaSystem(Params *p);
    ~AlphaSystem();

/**
 * Serialization stuff
 */
  public:
    virtual void serialize(std::ostream &os);
    virtual void unserialize(Checkpoint *cp, const std::string &section);

    /**
     * Set the m5AlphaAccess pointer in the console
     */
    void setAlphaAccess(Addr access);

    /** console symbol table */
    SymbolTable *consoleSymtab;

    /** pal symbol table */
    SymbolTable *palSymtab;

    /** Object pointer for the console code */
    ObjectFile *console;

    /** Object pointer for the PAL code */
    ObjectFile *pal;

#ifndef NDEBUG
    /** Event to halt the simulator if the console calls panic() */
    BreakPCEvent *consolePanicEvent;
#endif
  protected:
    const Params *params() const { return (const Params *)_params; }

    /** Add a function-based event to PALcode. */
    template <class T>
    T *addPalFuncEvent(const char *lbl)
    {
        return addFuncEvent<T>(palSymtab, lbl);
    }

    /** Add a function-based event to the console code. */
    template <class T>
    T *addConsoleFuncEvent(const char *lbl)
    {
        return addFuncEvent<T>(consoleSymtab, lbl);
    }

    virtual Addr fixFuncEventAddr(Addr addr);

};

#endif

