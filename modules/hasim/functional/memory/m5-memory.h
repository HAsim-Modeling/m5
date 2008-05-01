//
// INTEL CONFIDENTIAL
// Copyright (c) 2008 Intel Corp.  Recipient is granted a non-sublicensable 
// copyright license under Intel copyrights to copy and distribute this code 
// internally only. This code is provided "AS IS" with no support and with no 
// warranties of any kind, including warranties of MERCHANTABILITY,
// FITNESS FOR ANY PARTICULAR PURPOSE or INTELLECTUAL PROPERTY INFRINGEMENT. 
// By making any use of this code, Recipient agrees that no other licenses 
// to any Intel patents, trade secrets, copyrights or other intellectual 
// property rights are granted herein, and no other licenses shall arise by 
// estoppel, implication or by operation of law. Recipient accepts all risks 
// of use.
//
 
//
// @file m5_memory.h
// @brief ISA-independent m5 memory access
// @author Michael Adler
//

#ifndef __HASIM_M5_MEMORY__
#define __HASIM_M5_MEMORY__

#include "asim/syntax.h"
#include "asim/provides/m5_hasim_base.h"

// m5 includes
#include "config/full_system.hh"
#include "mem/port.hh"
#include "mem/translating_port.hh"

typedef class FUNCP_SIMULATED_MEMORY_CLASS *FUNCP_SIMULATED_MEMORY;

class FUNCP_SIMULATED_MEMORY_CLASS : public M5_HASIM_BASE_CLASS
{
  public:
    //
    // Required public interface
    //

    FUNCP_SIMULATED_MEMORY_CLASS();
    ~FUNCP_SIMULATED_MEMORY_CLASS();

    void Read(UINT64 addr, UINT64 size, void *dest);
    void Write(UINT64 addr, UINT64 size, void *src);

  private:
    TranslatingPort *mem_port;
    PageTable *pTable;
};

#endif //  __HASIM_M5_MEMORY__
