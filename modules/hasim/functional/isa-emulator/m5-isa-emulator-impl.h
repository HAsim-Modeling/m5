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
 
#ifndef _M5_ISA_EMULATOR_IMPL_
#define _M5_ISA_EMULATOR_IMPL_

#include <stdio.h>

#include "asim/syntax.h"

#include "asim/provides/funcp_base_types.h"
#include "asim/provides/hasim_isa.h"
#include "asim/provides/isa_emulator.h"
#include "asim/provides/m5_hasim_base.h"

// m5 includes
#include "config/full_system.hh"

// this module provides both client and service functionalities
typedef class ISA_EMULATOR_IMPL_CLASS* ISA_EMULATOR_IMPL;

class ISA_EMULATOR_IMPL_CLASS : public M5_HASIM_BASE_CLASS
{
  public:
    ISA_EMULATOR_IMPL_CLASS(ISA_EMULATOR parent);
    ~ISA_EMULATOR_IMPL_CLASS() {};

    void SyncReg(ISA_REG_INDEX_CLASS rName, FUNCP_INT_REG rVal);

    ISA_EMULATOR_RESULT Emulate(
        FUNCP_ADDR pc,
        ISA_INSTRUCTION inst,
        FUNCP_ADDR *newPC);

  private:
    ISA_EMULATOR_RESULT StartProgram(
        FUNCP_ADDR *newPC);

    ISA_EMULATOR parent;
    bool didInit;

    FUNCP_INT_REG intRegCache[TheISA::NumIntArchRegs];
};

#endif // _M5_ISA_EMULATOR_IMPL_
