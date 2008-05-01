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
// @file m5_hasim_base.h
// @brief Low level interface to m5 simulator
// @author Michael Adler
//

#ifndef __HASIM_M5_BASE__
#define __HASIM_M5_BASE__

#include "asim/syntax.h"
#include "asim/mesg.h"
#include "asim/atomic.h"

// m5
#include "cpu/simple/atomic.hh"

typedef class M5_HASIM_BASE_CLASS *M5_HASIM_BASE;

class M5_HASIM_BASE_CLASS
{
  public:
    M5_HASIM_BASE_CLASS();
    ~M5_HASIM_BASE_CLASS();

  protected:
    AtomicSimpleCPU *M5Cpu(UINT32 cpuId) const
    {
        ASSERTX(cpuId == 0);
        return m5cpu;
    };

  private:
    static ATOMIC32_CLASS refCnt;
    AtomicSimpleCPU *m5cpu;
};

#endif //  __HASIM_M5_BASE__
