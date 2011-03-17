/*
 * Copyright (c) 2002, 2003, 2004, 2005
 * The Regents of The University of Michigan
 * All Rights Reserved
 *
 * This code is part of the M5 simulator.
 *
 * Permission is granted to use, copy, create derivative works and
 * redistribute this software and such derivative works for any
 * purpose, so long as the copyright notice above, this grant of
 * permission, and the disclaimer below appear in all copies made; and
 * so long as the name of The University of Michigan is not used in
 * any advertising or publicity pertaining to the use or distribution
 * of this software without specific, written prior authorization.
 *
 * THIS SOFTWARE IS PROVIDED AS IS, WITHOUT REPRESENTATION FROM THE
 * UNIVERSITY OF MICHIGAN AS TO ITS FITNESS FOR ANY PURPOSE, AND
 * WITHOUT WARRANTY BY THE UNIVERSITY OF MICHIGAN OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE. THE REGENTS OF THE UNIVERSITY OF MICHIGAN SHALL NOT BE
 * LIABLE FOR ANY DAMAGES, INCLUDING DIRECT, SPECIAL, INDIRECT,
 * INCIDENTAL, OR CONSEQUENTIAL DAMAGES, WITH RESPECT TO ANY CLAIM
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OF THE SOFTWARE, EVEN
 * IF IT HAS BEEN OR IS HEREAFTER ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGES.
 *
 * Authors: Ali G. Saidi
 */

#include <string>

#include "arch/sparc/vtophys.hh"
#include "arch/sparc/tlb.hh"
#include "base/compiler.hh"
#include "base/chunk_generator.hh"
#include "base/trace.hh"
#include "cpu/thread_context.hh"
#include "mem/vport.hh"

using namespace std;

namespace SparcISA
{
    Addr vtophys(Addr vaddr)
    {
        // In SPARC it's almost always impossible to turn a VA->PA w/o a context
        // The only times we can kinda do it are if we have a SegKPM mapping
        // and can find the real address in the tlb or we have a physical
        // adddress already (beacuse we are looking at the hypervisor)
        // Either case is rare, so we'll just panic.

        panic("vtophys() without context on SPARC largly worthless\n");
        M5_DUMMY_RETURN
    }

    Addr vtophys(ThreadContext *tc, Addr addr)
    {
        // Here we have many options and are really implementing something like
        // a fill handler to find the address since there isn't a multilevel
        // table for us to walk around.
        //
        // 1. We are currently hyperpriv, return the address unmodified
        // 2. The mmu is off return(ra->pa)
        // 3. We are currently priv, use ctx0* tsbs to find the page
        // 4. We are not priv, use ctxN0* tsbs to find the page
        // For all accesses we check the tlbs first since it's possible that
        // long standing pages (e.g. locked kernel mappings) won't be in the tsb
        uint64_t tlbdata = tc->readMiscRegNoEffect(MISCREG_TLB_DATA);

        bool hpriv = bits(tlbdata,0,0);
        //bool priv = bits(tlbdata,2,2);
        bool addr_mask = bits(tlbdata,3,3);
        bool data_real = !bits(tlbdata,5,5);
        bool inst_real = !bits(tlbdata,4,4);
        bool ctx_zero  = bits(tlbdata,18,16) > 0;
        int part_id = bits(tlbdata,15,8);
        int pri_context = bits(tlbdata,47,32);
        //int sec_context = bits(tlbdata,63,48);

        FunctionalPort *mem = tc->getPhysPort();
        ITB* itb = tc->getITBPtr();
        DTB* dtb = tc->getDTBPtr();
        TlbEntry* tbe;
        PageTableEntry pte;
        Addr tsbs[4];
        Addr va_tag;
        TteTag ttetag;

        if (hpriv)
            return addr;

        if (addr_mask)
            addr = addr & VAddrAMask;

        tbe = dtb->lookup(addr, part_id, data_real, ctx_zero ? 0 : pri_context , false);
        if (tbe) goto foundtbe;

        tbe = itb->lookup(addr, part_id, inst_real, ctx_zero ? 0 : pri_context, false);
        if (tbe) goto foundtbe;

        // We didn't find it in the tlbs, so lets look at the TSBs
        dtb->GetTsbPtr(tc, addr, ctx_zero ? 0 : pri_context, tsbs);
        va_tag = bits(addr, 63, 22);
        for (int x = 0; x < 4; x++) {
            ttetag = betoh(mem->read<uint64_t>(tsbs[x]));
            if (ttetag.valid() && ttetag.va() == va_tag) {
                pte.populate(betoh(mem->read<uint64_t>(tsbs[x]) + sizeof(uint64_t)),
                        PageTableEntry::sun4v); // I think it's sun4v at least!
                DPRINTF(VtoPhys, "Virtual(%#x)->Physical(%#x) found in TTE\n", addr,
                        pte.paddrMask() | addr & pte.sizeMask());
                goto foundpte;
            }
        }
        panic("couldn't translate %#x\n", addr);

foundtbe:
        pte = tbe->pte;
        DPRINTF(VtoPhys, "Virtual(%#x)->Physical(%#x) found in TLB\n", addr,
                pte.paddrMask() | addr & pte.sizeMask());
foundpte:
        return pte.paddrMask() | addr & pte.sizeMask();
    }
}
