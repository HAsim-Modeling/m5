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
