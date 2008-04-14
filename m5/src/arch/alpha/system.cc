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

#include <sys/signal.h>

#include "arch/alpha/ev5.hh"
#include "arch/alpha/system.hh"
#include "arch/alpha/remote_gdb.hh"
#include "arch/vtophys.hh"
#include "base/loader/object_file.hh"
#include "base/loader/symtab.hh"
#include "base/trace.hh"
#include "mem/physical.hh"
#include "params/AlphaSystem.hh"
#include "sim/byteswap.hh"


using namespace LittleEndianGuest;

AlphaSystem::AlphaSystem(Params *p)
    : System(p)
{
    consoleSymtab = new SymbolTable;
    palSymtab = new SymbolTable;


    /**
     * Load the pal, and console code into memory
     */
    // Load Console Code
    console = createObjectFile(params()->console);
    if (console == NULL)
        fatal("Could not load console file %s", params()->console);

    // Load pal file
    pal = createObjectFile(params()->pal);
    if (pal == NULL)
        fatal("Could not load PALcode file %s", params()->pal);


    // Load program sections into memory
    pal->loadSections(&functionalPort, AlphaISA::LoadAddrMask);
    console->loadSections(&functionalPort, AlphaISA::LoadAddrMask);

    // load symbols
    if (!console->loadGlobalSymbols(consoleSymtab))
        panic("could not load console symbols\n");

    if (!pal->loadGlobalSymbols(palSymtab))
        panic("could not load pal symbols\n");

    if (!pal->loadLocalSymbols(palSymtab))
        panic("could not load pal symbols\n");

    if (!console->loadGlobalSymbols(debugSymbolTable))
        panic("could not load console symbols\n");

    if (!pal->loadGlobalSymbols(debugSymbolTable))
        panic("could not load pal symbols\n");

    if (!pal->loadLocalSymbols(debugSymbolTable))
        panic("could not load pal symbols\n");

     Addr addr = 0;
#ifndef NDEBUG
    consolePanicEvent = addConsoleFuncEvent<BreakPCEvent>("panic");
#endif

    /**
     * Copy the osflags (kernel arguments) into the consoles
     * memory. (Presently Linux does not use the console service
     * routine to get these command line arguments, but Tru64 and
     * others do.)
     */
    if (consoleSymtab->findAddress("env_booted_osflags", addr)) {
        virtPort.writeBlob(addr, (uint8_t*)params()->boot_osflags.c_str(),
                strlen(params()->boot_osflags.c_str()));
    }

    /**
     * Set the hardware reset parameter block system type and revision
     * information to Tsunami.
     */
    if (consoleSymtab->findAddress("m5_rpb", addr)) {
        uint64_t data;
        data = htog(params()->system_type);
        virtPort.write(addr+0x50, data);
        data = htog(params()->system_rev);
        virtPort.write(addr+0x58, data);
    } else
        panic("could not find hwrpb\n");

}

AlphaSystem::~AlphaSystem()
{
    delete consoleSymtab;
    delete console;
    delete pal;
#ifdef DEBUG
    delete consolePanicEvent;
#endif
}

/**
 * This function fixes up addresses that are used to match PCs for
 * hooking simulator events on to target function executions.
 *
 * Alpha binaries may have multiple global offset table (GOT)
 * sections.  A function that uses the GOT starts with a
 * two-instruction prolog which sets the global pointer (gp == r29) to
 * the appropriate GOT section.  The proper gp value is calculated
 * based on the function address, which must be passed by the caller
 * in the procedure value register (pv aka t12 == r27).  This sequence
 * looks like the following:
 *
 *			opcode Ra Rb offset
 *	ldah gp,X(pv)     09   29 27   X
 *	lda  gp,Y(gp)     08   29 29   Y
 *
 * for some constant offsets X and Y.  The catch is that the linker
 * (or maybe even the compiler, I'm not sure) may recognize that the
 * caller and callee are using the same GOT section, making this
 * prolog redundant, and modify the call target to skip these
 * instructions.  If we check for execution of the first instruction
 * of a function (the one the symbol points to) to detect when to skip
 * it, we'll miss all these modified calls.  It might work to
 * unconditionally check for the third instruction, but not all
 * functions have this prolog, and there's some chance that those
 * first two instructions could have undesired consequences.  So we do
 * the Right Thing and pattern-match the first two instructions of the
 * function to decide where to patch.
 *
 * Eventually this code should be moved into an ISA-specific file.
 */
Addr
AlphaSystem::fixFuncEventAddr(Addr addr)
{
    // mask for just the opcode, Ra, and Rb fields (not the offset)
    const uint32_t inst_mask = 0xffff0000;
    // ldah gp,X(pv): opcode 9, Ra = 29, Rb = 27
    const uint32_t gp_ldah_pattern = (9 << 26) | (29 << 21) | (27 << 16);
    // lda  gp,Y(gp): opcode 8, Ra = 29, rb = 29
    const uint32_t gp_lda_pattern  = (8 << 26) | (29 << 21) | (29 << 16);

    uint32_t i1 = virtPort.read<uint32_t>(addr);
    uint32_t i2 = virtPort.read<uint32_t>(addr + sizeof(AlphaISA::MachInst));

    if ((i1 & inst_mask) == gp_ldah_pattern &&
        (i2 & inst_mask) == gp_lda_pattern) {
        Addr new_addr = addr + 2* sizeof(AlphaISA::MachInst);
        DPRINTF(Loader, "fixFuncEventAddr: %p -> %p", addr, new_addr);
        return new_addr;
    } else {
        return addr;
    }
}


void
AlphaSystem::setAlphaAccess(Addr access)
{
    Addr addr = 0;
    if (consoleSymtab->findAddress("m5AlphaAccess", addr)) {
        virtPort.write(addr, htog(EV5::Phys2K0Seg(access)));
    } else
        panic("could not find m5AlphaAccess\n");
}

void
AlphaSystem::serialize(std::ostream &os)
{
    System::serialize(os);
    consoleSymtab->serialize("console_symtab", os);
    palSymtab->serialize("pal_symtab", os);
}


void
AlphaSystem::unserialize(Checkpoint *cp, const std::string &section)
{
    System::unserialize(cp,section);
    consoleSymtab->unserialize("console_symtab", cp, section);
    palSymtab->unserialize("pal_symtab", cp, section);
}

AlphaSystem *
AlphaSystemParams::create()
{
    return new AlphaSystem(this);
}
