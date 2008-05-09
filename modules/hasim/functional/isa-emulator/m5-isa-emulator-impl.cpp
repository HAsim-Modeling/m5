/* INTEL CONFIDENTIAL
 * Copyright (c) 2008 Intel Corp.  Recipient is granted a non-sublicensable 
 * copyright license under Intel copyrights to copy and distribute this code 
 * internally only. This code is provided "AS IS" with no support and with no 
 * warranties of any kind, including warranties of MERCHANTABILITY,
 * FITNESS FOR ANY PARTICULAR PURPOSE or INTELLECTUAL PROPERTY INFRINGEMENT. 
 * By making any use of this code, Recipient agrees that no other licenses 
 * to any Intel patents, trade secrets, copyrights or other intellectual 
 * property rights are granted herein, and no other licenses shall arise by 
 * estoppel, implication or by operation of law. Recipient accepts all risks 
 * of use.
*/
 
#include "asim/syntax.h"

#include "asim/provides/isa_emulator_impl.h"

ISA_EMULATOR_IMPL_CLASS::ISA_EMULATOR_IMPL_CLASS(
    ISA_EMULATOR parent) :
    parent(parent),
    didInit(false)
{
}


void
ISA_EMULATOR_IMPL_CLASS::SyncReg(
    ISA_REG_INDEX_CLASS rName,
    FUNCP_INT_REG rVal)
{
    //
    // Skip register sync until the hardware has asked to emulate one instruction.
    // It will start simulation at address 0x1000, which holds 0's.
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
    FUNCP_ADDR pc,
    ISA_INSTRUCTION inst,
    FUNCP_ADDR *newPC)
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
        ASIMWARNING("HALT instruction executed.  Probably a bug.")
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

    //
    // Is the instruction a branch?
    //
    Addr branchTarget;
    bool isBranch = cpu->curStaticInst->isControl();
    if (isBranch)
    {
        cpu->curStaticInst->hasBranchTarget(pc, cpu->tc, branchTarget);
    }

    Fault fault = cpu->curStaticInst->execute(cpu, NULL);
    VERIFY(fault == NoFault, "Fault emulating instr at 0x" << fmt_x(pc) << " in m5");

    cpu->postExecute();
    VERIFYX(! cpu->stayAtPC);

    cpu->advancePC(fault);

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
    FUNCP_ADDR *newPC)
{
    ASSERTX(sizeof(ISA_INSTRUCTION) == sizeof(TheISA::MachInst));

    //
    // Startup sequence.  HAsim model starts at PC 0x1000.  m5 returns 0
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
