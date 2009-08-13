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
static CONTEXT_ID emulationCtxId = 0;

void
HAsimEmulMemoryRead(Addr paddr, UINT64 size)
{
    if (inEmulation)
    {
        inEmulation = false;    // Prevent loops
        FUNCP_MEMORY_CLASS::NoteSystemMemoryRead(emulationCtxId, paddr, size);
        inEmulation = true;
    }
}

void
HAsimEmulMemoryWrite(Addr paddr, UINT64 size)
{
    if (inEmulation)
    {
        inEmulation = false;    // Prevent loops
        FUNCP_MEMORY_CLASS::NoteSystemMemoryWrite(emulationCtxId, paddr, size);
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
    parent(parent)
{
    HAsimNoteMemoryRead = &HAsimEmulMemoryRead;
    HAsimNoteMemoryWrite = &HAsimEmulMemoryWrite;

    didInit = new bool[NumCPUs()];
    skewCnt = new UINT32[NumCPUs()];
    for (UINT32 c = 0; c < NumCPUs(); c++)
    {
        didInit[c] = false;
        skewCnt[c] = c * SKEW_CONTEXTS;
    }
}


ISA_EMULATOR_IMPL_CLASS::~ISA_EMULATOR_IMPL_CLASS()
{
    delete[] didInit;
    delete[] skewCnt;
}


void
ISA_EMULATOR_IMPL_CLASS::SyncReg(
    CONTEXT_ID ctxId,
    ISA_REG_INDEX_CLASS rName,
    FUNCP_REG rVal)
{
    //
    // Skip register sync until the hardware has asked to emulate one instruction.
    // It will start simulation at address 0, which holds 0's.
    //
    if (! didInit[ctxId]) return;

    if (rName.IsArchReg())
    {
        M5Cpu(ctxId)->tc->setIntReg(rName.ArchRegNum(), rVal.intReg);

        ASSERTX(rName.ArchRegNum() < TheISA::NumIntArchRegs);
        intRegCache[rName.ArchRegNum()] = rVal.intReg;
    }

    if (rName.IsFPReg())
    {
        M5Cpu(ctxId)->tc->setFloatReg(rName.FPRegNum(), rVal.fpReg);

        ASSERTX(rName.FPRegNum() < TheISA::NumFloatArchRegs);
        fpRegCache[rName.FPRegNum()] = rVal.fpReg;
    }
}


ISA_EMULATOR_RESULT
ISA_EMULATOR_IMPL_CLASS::Emulate(
    CONTEXT_ID ctxId,
    FUNCP_VADDR pc,
    ISA_INSTRUCTION inst,
    FUNCP_VADDR *newPC)
{
    if (! didInit[ctxId])
    {
        return StartProgram(ctxId, pc, newPC);
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

    AtomicSimpleCPU *cpu = M5Cpu(ctxId);

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
    emulationCtxId = ctxId;

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
        fault = cpu->curStaticInst->execute(cpu, cpu->getTraceData());
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
        FUNCP_INT_REG rVal = M5Cpu(ctxId)->tc->readIntReg(r);
        if (intRegCache[r] != rVal)
        {
            FUNCP_REG v;
            v.intReg = rVal;
            parent->UpdateRegister(ctxId, rName, v);
            intRegCache[r] = rVal;
        }
    }

    for (int r = 0; r < TheISA::NumFloatArchRegs; r++)
    {
        ISA_REG_INDEX_CLASS rName;
        rName.SetFPReg(r);
        FUNCP_FP_REG rVal = M5Cpu(ctxId)->tc->readFloatReg(r);
        if (fpRegCache[r] != rVal)
        {
            FUNCP_REG v;
            v.fpReg = rVal;
            parent->UpdateRegister(ctxId, rName, v);
            fpRegCache[r] = rVal;
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
    CONTEXT_ID ctxId,
    FUNCP_VADDR curPC,
    FUNCP_VADDR *newPC)
{
    ASSERTX(sizeof(ISA_INSTRUCTION) == sizeof(TheISA::MachInst));

    //
    // Skewed start forces contexts to loop back to the start PC for some
    // number of instructions so that the contexts won't all be doing the
    // same thing every cycle when each context is running the same workload.
    //

    if (skewCnt[ctxId] != 0)
    {
        //
        // Not ready to start this context.  Loop back to same PC.
        //
        *newPC = curPC;
        skewCnt[ctxId] -= 1;
        return ISA_EMULATOR_BRANCH;
    }
    else
    {
        //
        // Startup sequence.  HAsim model starts at PC 0.  m5 returns 0
        // for the instruction.  HAsim calls here to emulate the instruction.
        // Now set all the start register values and jump to the right PC.
        //

        for (int r = 0; r < TheISA::NumIntArchRegs; r++)
        {
            ISA_REG_INDEX_CLASS rName;
            FUNCP_REG rVal;
            rName.SetArchReg(r);
            rVal.intReg = M5Cpu(ctxId)->tc->readIntReg(r);
            parent->UpdateRegister(ctxId, rName, rVal);
            intRegCache[r] = rVal.intReg;
        }

        for (int r = 0; r < TheISA::NumIntArchRegs; r++)
        {
            ISA_REG_INDEX_CLASS rName;
            FUNCP_REG rVal;
            rName.SetFPReg(r);
            rVal.fpReg = M5Cpu(ctxId)->tc->readFloatReg(r);
            parent->UpdateRegister(ctxId, rName, rVal);
            fpRegCache[r] = rVal.fpReg;
        }

        *newPC = M5Cpu(ctxId)->readPC();
        didInit[ctxId] = true;
        return ISA_EMULATOR_BRANCH;
    }
}
