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
static bool emulationMayRefMemory = false;
static CONTEXT_ID emulationCtxId = 0;

void
HAsimEmulMemoryRead(Addr paddr, UINT64 size)
{
    if (inEmulation)
    {
        // Some emulation modes assume no memory is referenced
        VERIFY(emulationMayRefMemory, "Emulated REGOP touches memory!");

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
        // Some emulation modes assume no memory is referenced
        VERIFY(emulationMayRefMemory, "Emulated REGOP touches memory!");

        inEmulation = false;    // Prevent loops
        FUNCP_MEMORY_CLASS::NoteSystemMemoryWrite(emulationCtxId, paddr, size);
        inEmulation = true;
    }
}


// ========================================================================
//
// m5 Full Instruction Emulation...
//
// ========================================================================

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
    emulationMayRefMemory = true;

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
        // Not ready to start this context.  Loop back to same PC for a while, then check again.
        //
        *newPC = curPC;
        skewCnt[ctxId] -= 1;
        return ISA_EMULATOR_SLEEP;
    }
    else
    {
        //
        // Startup sequence.  HAsim model starts at PC 0.  m5 returns 0
        // for the instruction.  HAsim calls here to emulate the instruction.
        // If the PC is non-0 in M5 then the context is ready to start.
        // Otherwise tell it to sleep for a while and then check again.

        *newPC = M5Cpu(ctxId)->readPC();
        didInit[ctxId] = (*newPC != 0);
        
        if (didInit[ctxId])
        {

            //
            // Context is ready to start.
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

            ASIMWARNING("Activating Context: " << (int) ctxId << endl);
            return ISA_EMULATOR_BRANCH;
        }
        else
        {
        
            // Context is not ready. Tell the functional partition to sleep
            // this context for a while, and then check again.
            return ISA_EMULATOR_SLEEP;
        }

    }
}


// ========================================================================
//
// m5 Operation-Level Emulation...
//
// ========================================================================

ISA_REGOP_EMULATOR_IMPL_CLASS::ISA_REGOP_EMULATOR_IMPL_CLASS(
    ISA_REGOP_EMULATOR parent) :
    parent(parent)
{}


ISA_REGOP_EMULATOR_IMPL_CLASS::~ISA_REGOP_EMULATOR_IMPL_CLASS()
{}


FUNCP_REG
ISA_REGOP_EMULATOR_IMPL_CLASS::EmulateRegOp(
    CONTEXT_ID ctxId,
    FUNCP_VADDR pc,
    ISA_INSTRUCTION inst,
    FUNCP_REG srcVal0,
    FUNCP_REG srcVal1,
    ISA_REG_INDEX_CLASS rNameSrc0,
    ISA_REG_INDEX_CLASS rNameSrc1,
    ISA_REG_INDEX_CLASS rNameDst)
{
    if (rNameSrc0.IsArchReg())
    {
        M5Cpu(ctxId)->tc->setIntReg(rNameSrc0.ArchRegNum(), srcVal0.intReg);
    }

    if (rNameSrc0.IsFPReg())
    {
        M5Cpu(ctxId)->tc->setFloatReg(rNameSrc0.FPRegNum(), srcVal0.fpReg);
    }

    if (rNameSrc1.IsArchReg())
    {
        M5Cpu(ctxId)->tc->setIntReg(rNameSrc1.ArchRegNum(), srcVal1.intReg);
    }

    if (rNameSrc1.IsFPReg())
    {
        M5Cpu(ctxId)->tc->setFloatReg(rNameSrc1.FPRegNum(), srcVal1.fpReg);
    }


    //
    // Similar to Emulate() above but restricts the instruction to non-branch,
    // no faults and no memory.
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
    emulationMayRefMemory = false;
    emulationCtxId = ctxId;

    //
    // Is the instruction a branch?
    //
    bool isBranch = cpu->curStaticInst->isControl();
    Fault fault = cpu->curStaticInst->execute(cpu, cpu->getTraceData());

    VERIFY((fault == NoFault), "RegOP Emulator triggered a fault!");
    VERIFY(! isBranch, "RegOP attempted emulation of branch!");

    cpu->postExecute();
    VERIFYX(! cpu->stayAtPC);

    cpu->advancePC(fault);

    // Stop watching memory
    inEmulation = false;

    FUNCP_REG rVal;
    if (rNameDst.IsArchReg())
    {
        rVal.intReg = cpu->tc->readIntReg(rNameDst.ArchRegNum());
    }
    else if (rNameDst.IsFPReg())
    {
        rVal.fpReg = cpu->tc->readFloatReg(rNameDst.FPRegNum());
    }
    else
    {
        ASIMERROR("Unexpected register type");
    }

    return rVal;
}
