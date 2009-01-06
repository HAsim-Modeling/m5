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
// @file m5_memory.h
// @brief ISA-independent m5 memory access
// @author Michael Adler
//

#ifndef __HASIM_M5_MEMORY__
#define __HASIM_M5_MEMORY__

#include "asim/syntax.h"
#include "asim/provides/m5_hasim_base.h"
#include "asim/provides/funcp_base_types.h"

// m5 includes
#include "config/full_system.hh"
#include "mem/port.hh"
#include "mem/translating_port.hh"

typedef class FUNCP_SIMULATED_MEMORY_CLASS *FUNCP_SIMULATED_MEMORY;

// Response from VtoP
struct FUNCP_MEM_VTOP_RESP
{
    UINT64 pa;
    bool pageFault;    // Translation failed
    bool ioSpace;      // Reference is to uncacheable I/O space
};

class FUNCP_SIMULATED_MEMORY_CLASS : public M5_HASIM_BASE_CLASS,
                                     public TRACEABLE_CLASS
{
  public:
    //
    // Required public interface
    //

    FUNCP_SIMULATED_MEMORY_CLASS();
    ~FUNCP_SIMULATED_MEMORY_CLASS();

    void Read(CONTEXT_ID ctx_id, UINT64 paddr, UINT64 size, void *dest);
    void Write(CONTEXT_ID ctx_id, UINT64 paddr, UINT64 size, void *src);

    FUNCP_MEM_VTOP_RESP VtoP(CONTEXT_ID ctx_id, UINT64 va, bool allocOnFault);

  private:
    TranslatingPort *mem_port;
    PageTable *pTable;
    Format fmt_va;

    Addr guard_page;        // Mapped to virtual address 0

    void BlobHelper(Addr paddr, uint8_t *p, int size, MemCmd cmd);
};

#endif //  __HASIM_M5_MEMORY__
