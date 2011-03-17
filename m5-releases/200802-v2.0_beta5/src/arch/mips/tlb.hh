/*
 * Copyright (c) 2001, 2002, 2003, 2004, 2005
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
 * Authors: Nathan L. Binkert
 *          Steven K. Reinhardt
 */

/*
 * Copyright (c) 2007 MIPS Technologies, Inc.  All Rights Reserved
 *
 * This software is part of the M5 simulator.
 *
 * THIS IS A LEGAL AGREEMENT.  BY DOWNLOADING, USING, COPYING, CREATING
 * DERIVATIVE WORKS, AND/OR DISTRIBUTING THIS SOFTWARE YOU ARE AGREEING
 * TO THESE TERMS AND CONDITIONS.
 *
 * Permission is granted to use, copy, create derivative works and
 * distribute this software and such derivative works for any purpose,
 * so long as (1) the copyright notice above, this grant of permission,
 * and the disclaimer below appear in all copies and derivative works
 * made, (2) the copyright notice above is augmented as appropriate to
 * reflect the addition of any new copyrightable work in a derivative
 * work (e.g., Copyright (c) <Publication Year> Copyright Owner), and (3)
 * the name of MIPS Technologies, Inc. (“MIPS”) is not used in any
 * advertising or publicity pertaining to the use or distribution of
 * this software without specific, written prior authorization.
 *
 * THIS SOFTWARE IS PROVIDED “AS IS.”  MIPS MAKES NO WARRANTIES AND
 * DISCLAIMS ALL WARRANTIES, WHETHER EXPRESS, STATUTORY, IMPLIED OR
 * OTHERWISE, INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND
 * NON-INFRINGEMENT OF THIRD PARTY RIGHTS, REGARDING THIS SOFTWARE.
 * IN NO EVENT SHALL MIPS BE LIABLE FOR ANY DAMAGES, INCLUDING DIRECT,
 * INDIRECT, INCIDENTAL, CONSEQUENTIAL, SPECIAL, OR PUNITIVE DAMAGES OF
 * ANY KIND OR NATURE, ARISING OUT OF OR IN CONNECTION WITH THIS AGREEMENT,
 * THIS SOFTWARE AND/OR THE USE OF THIS SOFTWARE, WHETHER SUCH LIABILITY
 * IS ASSERTED ON THE BASIS OF CONTRACT, TORT (INCLUDING NEGLIGENCE OR
 * STRICT LIABILITY), OR OTHERWISE, EVEN IF MIPS HAS BEEN WARNED OF THE
 * POSSIBILITY OF ANY SUCH LOSS OR DAMAGE IN ADVANCE.
 *
 * Authors: Jaidev P. Patwardhan
 *          Korey L. Sewell
 *
 */

#ifndef __ARCH_MIPS_TLB_HH__
#define __ARCH_MIPS_TLB_HH__

#include <map>

#include "arch/mips/isa_traits.hh"
#include "arch/mips/utility.hh"
#include "arch/mips/vtophys.hh"
#include "arch/mips/pagetable.hh"
#include "base/statistics.hh"
#include "mem/request.hh"
#include "params/MipsDTB.hh"
#include "params/MipsITB.hh"
#include "sim/faults.hh"
#include "sim/tlb.hh"
#include "sim/sim_object.hh"

class ThreadContext;

/* MIPS does not distinguish between a DTLB and an ITLB -> unified TLB
   However, to maintain compatibility with other architectures, we'll
   simply create an ITLB and DTLB that will point to the real TLB */
namespace MipsISA {

// WARN: This particular TLB entry is not necessarily conformed to MIPS ISA
struct TlbEntry
{
    Addr _pageStart;
    TlbEntry() {}
    TlbEntry(Addr asn, Addr vaddr, Addr paddr) : _pageStart(paddr) {}

    Addr pageStart()
    {
        return _pageStart;
    }

    void serialize(std::ostream &os)
    {
        SERIALIZE_SCALAR(_pageStart);
    }

    void unserialize(Checkpoint *cp, const std::string &section)
    {
        UNSERIALIZE_SCALAR(_pageStart);
    }

};

class TLB : public BaseTLB
{
  protected:
    typedef std::multimap<Addr, int> PageTable;
    PageTable lookupTable;	// Quick lookup into page table

    MipsISA::PTE *table;	// the Page Table
    int size;			// TLB Size
    int nlu;			// not last used entry (for replacement)

    void nextnlu() { if (++nlu >= size) nlu = 0; }
    MipsISA::PTE *lookup(Addr vpn, uint8_t asn) const;

    mutable Stats::Scalar<> read_hits;
    mutable Stats::Scalar<> read_misses;
    mutable Stats::Scalar<> read_acv;
    mutable Stats::Scalar<> read_accesses;
    mutable Stats::Scalar<> write_hits;
    mutable Stats::Scalar<> write_misses;
    mutable Stats::Scalar<> write_acv;
    mutable Stats::Scalar<> write_accesses;
    Stats::Formula hits;
    Stats::Formula misses;
    Stats::Formula invalids;
    Stats::Formula accesses;

  public:
    typedef MipsTLBParams Params;
    TLB(const Params *p);

    int probeEntry(Addr vpn,uint8_t) const;
    MipsISA::PTE *getEntry(unsigned) const;
    virtual ~TLB();
    int smallPages;
    int getsize() const { return size; }

    MipsISA::PTE &index(bool advance = true);
    void insert(Addr vaddr, MipsISA::PTE &pte);
    void insertAt(MipsISA::PTE &pte, unsigned Index, int _smallPages);
    void flushAll();
    void demapPage(Addr vaddr, uint64_t asn)
    {
        panic("demapPage unimplemented.\n");
    }

    // static helper functions... really
    static bool validVirtualAddress(Addr vaddr);

    static Fault checkCacheability(RequestPtr &req);

    // Checkpointing
    void serialize(std::ostream &os);
    void unserialize(Checkpoint *cp, const std::string &section);

    void regStats();
};

class ITB : public TLB {
  public:
    typedef MipsTLBParams Params;
    ITB(const Params *p);

    Fault translate(RequestPtr &req, ThreadContext *tc);
};

class DTB : public TLB {
  public:
    typedef MipsTLBParams Params;
    DTB(const Params *p);

    Fault translate(RequestPtr &req, ThreadContext *tc, bool write = false);
};

class UTB : public ITB, public DTB {
  public:
    typedef MipsTLBParams Params;
    UTB(const Params *p);

};

}



#endif // __MIPS_MEMORY_HH__
