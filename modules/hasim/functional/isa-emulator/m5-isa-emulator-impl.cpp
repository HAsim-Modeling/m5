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
        //
        // Startup sequence.  HAsim model starts at PC 0x1000.  m5 returns 0
        // for the instruction.  HAsim calls here to emulate the instruction.
        // Now set all the start register values and jump to the right PC.
        //

        for (int r = 0; r < TheISA::NumIntArchRegs; r++)
        {
            ISA_REG_INDEX_CLASS rName;
            rName.SetArchReg(r);
            parent->UpdateRegister(rName, M5Cpu(0)->tc->readIntReg(r));
        }

        *newPC = M5Cpu(0)->readPC();
        didInit = true;
        return ISA_EMULATOR_BRANCH;
    }
    
    ASIMWARNING("m5 ISA emulation not yet working\n");

    *newPC = 0;
    return ISA_EMULATOR_EXIT_FAIL;
}

