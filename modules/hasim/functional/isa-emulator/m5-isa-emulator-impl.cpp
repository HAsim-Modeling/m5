//
// Copyright (C) 2008 Intel Corporation
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
 
#include "asim/syntax.h"

#include "asim/provides/isa_emulator_impl.h"
#include "asim/provides/funcp_memory.h"


//***********************************************************************
//
// Callbacks from m5's physical memory access routines so HAsim
// can monitor memory updates during emulation and invalid FPGA-side
// memory caches.
//
//***********************************************************************

extern void (*HAsimNoteMemoryRead)(Addr paddr, uint64_t size);
extern void (*HAsimNoteMemoryWrite)(Addr paddr, uint64_t size);

static bool inEmulation = false;

void
HAsimEmulMemoryRead(Addr paddr, UINT64 size)
{
    if (inEmulation)
    {
        inEmulation = false;    // Prevent loops
        FUNCP_MEMORY_CLASS::NoteSystemMemoryRead(paddr, size);
        inEmulation = true;
    }
}

void
HAsimEmulMemoryWrite(Addr paddr, UINT64 size)
{
    if (inEmulation)
    {
        inEmulation = false;    // Prevent loops
        FUNCP_MEMORY_CLASS::NoteSystemMemoryWrite(paddr, size);
        inEmulation = true;
    }
}


//***********************************************************************
//
// m5 Emulation...
//
//***********************************************************************

ISA_EMULATOR_IMPL_CLASS::ISA_EMULATOR_IMPL_CLASS(
    ISA_EMULATOR parent) :
    parent(parent),
    didInit(false)
{
    HAsimNoteMemoryRead = &HAsimEmulMemoryRead;
    HAsimNoteMemoryWrite = &HAsimEmulMemoryWrite;
}


void
ISA_EMULATOR_IMPL_CLASS::SyncReg(
    ISA_REG_INDEX_CLASS rName,
    FUNCP_INT_REG rVal)
{
    //
    // Skip register sync until the hardware has asked to emulate one instruction.
    // It will start simulation at address 0, which holds 0's.
    //
    if (! didInit) return;

    if (rName.IsArchReg())
    {
        M5Cpu(0)->tc->setIntReg(rName.ArchRegNum(), rVal);

        ASSERTX(rName.ArchRegNum() < TheISA::NumIntArchRegs);
        intRegCache[rName.ArchRegNum()] = rVal;
    }
}


ISA_EMULATOR_RESULT
ISA_EMULATOR_IMPL_CLASS::Emulate(
    FUNCP_VADDR pc,
    ISA_INSTRUCTION inst,
    FUNCP_VADDR *newPC)
{
    if (! didInit)
    {
        return StartProgram(newPC);
    }

#if THE_ISA == ALPHA_ISA
    //
    // HALT?  Return an error
    //
    if (inst == 0)
    {
        ASIMWARNING("HALT instruction executed.  Probably a bug." << endl)
        *newPC = 0;
        return ISA_EMULATOR_EXIT_FAIL;
    }
#endif

    //
    // Set the m5 state and emulate a tick.  The code below is derived from
    // m5's AtomicSimpleCPU::tick()
    //

    AtomicSimpleCPU *cpu = M5Cpu(0);

    // m5 better not be in the middle of an instruction
    VERIFYX(! cpu->curMacroStaticInst);

    //
    // Set the machine state and execute the instruction
    //
    cpu->setPC(pc);
    cpu->setNextPC(pc + sizeof(TheISA::MachInst));
    cpu->inst = inst;
    cpu->preExecute();

    VERIFYX(cpu->curStaticInst);

    // Start watching memory
    inEmulation = true;

    //
    // Is the instruction a branch?
    //
    Addr branchTarget;
    bool isBranch = cpu->curStaticInst->isControl();
    if (isBranch)
    {
        cpu->curStaticInst->hasBranchTarget(pc, cpu->tc, branchTarget);
    }

    Fault fault;
    int fault_trips = 0;
    do
    {
        fault = cpu->curStaticInst->execute(cpu, NULL);
        if (fault != NoFault)
        {
            fault_trips += 1;
            VERIFY(fault_trips < 10, "Too many faults while emulating instr at 0x" << fmt_x(pc) << " in m5");
            fault->invoke(cpu->tc);
        }
    }
    while (fault != NoFault);

    cpu->postExecute();
    VERIFYX(! cpu->stayAtPC);

    cpu->advancePC(fault);

    // Stop watching memory
    inEmulation = false;

    //
    // Update registers
    //
    for (int r = 0; r < TheISA::NumIntArchRegs; r++)
    {
        ISA_REG_INDEX_CLASS rName;
        rName.SetArchReg(r);
        FUNCP_INT_REG rVal = M5Cpu(0)->tc->readIntReg(r);
        if (intRegCache[r] != rVal)
        {
            parent->UpdateRegister(rName, cpu->tc->readIntReg(r));
            intRegCache[r] = rVal;
        }
    }

    if (isBranch)
    {
        if (cpu->readPC() == branchTarget)
        {
            *newPC = cpu->readPC();
        }
        else
        {
            *newPC = 0;
            isBranch = false;
        }
    }

    if (cpu->tc->exitCalled())
    {
        return (cpu->tc->exitCode() == 0) ? ISA_EMULATOR_EXIT_OK : ISA_EMULATOR_EXIT_FAIL;
    }

    return isBranch ? ISA_EMULATOR_BRANCH : ISA_EMULATOR_NORMAL;
}


//
// Called once at the beginning of the program to set initial register state
// and the PC.
//
ISA_EMULATOR_RESULT
ISA_EMULATOR_IMPL_CLASS::StartProgram(
    FUNCP_VADDR *newPC)
{
    ASSERTX(sizeof(ISA_INSTRUCTION) == sizeof(TheISA::MachInst));

    //
    // Startup sequence.  HAsim model starts at PC 0.  m5 returns 0
    // for the instruction.  HAsim calls here to emulate the instruction.
    // Now set all the start register values and jump to the right PC.
    //

    for (int r = 0; r < TheISA::NumIntArchRegs; r++)
    {
        ISA_REG_INDEX_CLASS rName;
        rName.SetArchReg(r);
        FUNCP_INT_REG rVal = M5Cpu(0)->tc->readIntReg(r);
        parent->UpdateRegister(rName, rVal);
        intRegCache[r] = rVal;
    }

    *newPC = M5Cpu(0)->readPC();
    didInit = true;
    return ISA_EMULATOR_BRANCH;
}
