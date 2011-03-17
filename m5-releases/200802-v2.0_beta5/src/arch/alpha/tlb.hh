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

#ifndef __ALPHA_MEMORY_HH__
#define __ALPHA_MEMORY_HH__

#include <map>

#include "arch/alpha/ev5.hh"
#include "arch/alpha/isa_traits.hh"
#include "arch/alpha/pagetable.hh"
#include "arch/alpha/utility.hh"
#include "arch/alpha/vtophys.hh"
#include "base/statistics.hh"
#include "mem/request.hh"
#include "params/AlphaDTB.hh"
#include "params/AlphaITB.hh"
#include "sim/faults.hh"
#include "sim/tlb.hh"

class ThreadContext;

namespace AlphaISA
{
    class TlbEntry;

    class TLB : public BaseTLB
    {
      protected:
        typedef std::multimap<Addr, int> PageTable;
        PageTable lookupTable;  // Quick lookup into page table

        TlbEntry *table;        // the Page Table
        int size;               // TLB Size
        int nlu;                // not last used entry (for replacement)

        void nextnlu() { if (++nlu >= size) nlu = 0; }
        TlbEntry *lookup(Addr vpn, uint8_t asn);

      public:
        typedef AlphaTLBParams Params;
        TLB(const Params *p);
        virtual ~TLB();

        int getsize() const { return size; }

        TlbEntry &index(bool advance = true);
        void insert(Addr vaddr, TlbEntry &entry);

        void flushAll();
        void flushProcesses();
        void flushAddr(Addr addr, uint8_t asn);

        void demapPage(Addr vaddr, uint64_t asn)
        {
            assert(asn < (1 << 8));
            flushAddr(vaddr, asn);
        }

        // static helper functions... really EV5 VM traits
        static bool validVirtualAddress(Addr vaddr) {
            // unimplemented bits must be all 0 or all 1
            Addr unimplBits = vaddr & EV5::VAddrUnImplMask;
            return (unimplBits == 0) || (unimplBits == EV5::VAddrUnImplMask);
        }

        static Fault checkCacheability(RequestPtr &req);

        // Checkpointing
        virtual void serialize(std::ostream &os);
        virtual void unserialize(Checkpoint *cp, const std::string &section);

        // Most recently used page table entries
        TlbEntry *EntryCache[3];
        inline void flushCache()
        {
            memset(EntryCache, 0, 3 * sizeof(TlbEntry*));
        }

        inline TlbEntry* updateCache(TlbEntry *entry) {
            EntryCache[2] = EntryCache[1];
            EntryCache[1] = EntryCache[0];
            EntryCache[0] = entry;
            return entry;
        }
    };

    class ITB : public TLB
    {
      protected:
        mutable Stats::Scalar<> hits;
        mutable Stats::Scalar<> misses;
        mutable Stats::Scalar<> acv;
        mutable Stats::Formula accesses;

      public:
        typedef AlphaITBParams Params;
        ITB(const Params *p);
        virtual void regStats();

        Fault translate(RequestPtr &req, ThreadContext *tc);
    };

    class DTB : public TLB
    {
      protected:
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
        Stats::Formula acv;
        Stats::Formula accesses;

      public:
        typedef AlphaDTBParams Params;
        DTB(const Params *p);
        virtual void regStats();

        Fault translate(RequestPtr &req, ThreadContext *tc, bool write);
    };
}

#endif // __ALPHA_MEMORY_HH__
