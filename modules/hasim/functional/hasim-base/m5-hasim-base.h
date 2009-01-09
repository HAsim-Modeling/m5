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
typedef AtomicSimpleCPU *AtomicSimpleCPU_PTR;

class M5_HASIM_BASE_CLASS
{
  public:
    M5_HASIM_BASE_CLASS();
    ~M5_HASIM_BASE_CLASS();

    UINT32 NumCPUs() const { return numCPUs; };

  protected:
    AtomicSimpleCPU *M5Cpu(UINT32 cpuId) const
    {
        ASSERTX(cpuId < numCPUs);
        return m5cpus[cpuId];
    };

  private:
    static ATOMIC32_CLASS refCnt;
    static AtomicSimpleCPU_PTR *m5cpus;
    static UINT32 numCPUs;
};

#endif //  __HASIM_M5_BASE__
