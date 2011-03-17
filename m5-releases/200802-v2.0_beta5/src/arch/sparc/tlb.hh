/*
 * Copyright (c) 2006
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
 * Authors: Ali Saidi
 */

#ifndef __ARCH_SPARC_TLB_HH__
#define __ARCH_SPARC_TLB_HH__

#include "arch/sparc/asi.hh"
#include "arch/sparc/tlb_map.hh"
#include "base/misc.hh"
#include "config/full_system.hh"
#include "mem/request.hh"
#include "params/SparcDTB.hh"
#include "params/SparcITB.hh"
#include "sim/faults.hh"
#include "sim/tlb.hh"

class ThreadContext;
class Packet;

namespace SparcISA
{

class TLB : public BaseTLB
{
#if !FULL_SYSTEM
    //These faults need to be able to populate the tlb in SE mode.
    friend class FastInstructionAccessMMUMiss;
    friend class FastDataAccessMMUMiss;
#endif

    //TLB state
  protected:
    uint64_t c0_tsb_ps0;
    uint64_t c0_tsb_ps1;
    uint64_t c0_config;
    uint64_t cx_tsb_ps0;
    uint64_t cx_tsb_ps1;
    uint64_t cx_config;
    uint64_t sfsr;
    uint64_t tag_access;

  protected:
    TlbMap lookupTable;;
    typedef TlbMap::iterator MapIter;

    TlbEntry *tlb;

    int size;
    int usedEntries;
    int lastReplaced;

    uint64_t cacheState;
    bool cacheValid;

    std::list<TlbEntry*> freeList;

    enum FaultTypes {
        OtherFault = 0,
        PrivViolation = 0x1,
        SideEffect = 0x2,
        AtomicToIo = 0x4,
        IllegalAsi = 0x8,
        LoadFromNfo = 0x10,
        VaOutOfRange = 0x20,
        VaOutOfRangeJmp = 0x40
    };

    enum ContextType {
        Primary = 0,
        Secondary = 1,
        Nucleus = 2
    };

    enum TsbPageSize {
        Ps0,
        Ps1
    };
  public:
    /** lookup an entry in the TLB based on the partition id, and real bit if
     * real is true or the partition id, and context id if real is false.
     * @param va the virtual address not shifted (e.g. bottom 13 bits are 0)
     * @param paritition_id partition this entry is for
     * @param real is this a real->phys or virt->phys translation
     * @param context_id if this is virt->phys what context
     * @param update_used should ew update the used bits in the entries on not
     * useful if we are trying to do a va->pa without mucking with any state for
     * a debug read for example.
     * @return A pointer to a tlb entry
     */
    TlbEntry *lookup(Addr va, int partition_id, bool real, int context_id = 0,
            bool update_used = true);
  protected:
    /** Insert a PTE into the TLB. */
    void insert(Addr vpn, int partition_id, int context_id, bool real,
            const PageTableEntry& PTE, int entry = -1);

    /** Given an entry id, read that tlb entries' tag. */
    uint64_t TagRead(int entry);

    /** Remove all entries from the TLB */
    void invalidateAll();

    /** Remove all non-locked entries from the tlb that match partition id. */
    void demapAll(int partition_id);

    /** Remove all entries that match a given context/partition id. */
    void demapContext(int partition_id, int context_id);

    /** Remve all entries that match a certain partition id, (contextid), and
     * va). */
    void demapPage(Addr va, int partition_id, bool real, int context_id);

    /** Checks if the virtual address provided is a valid one. */
    bool validVirtualAddress(Addr va, bool am);

    void writeSfsr(bool write, ContextType ct,
            bool se, FaultTypes ft, int asi);

    void clearUsedBits();


    void writeTagAccess(Addr va, int context);

  public:
    typedef SparcTLBParams Params;
    TLB(const Params *p);

    void demapPage(Addr vaddr, uint64_t asn)
    {
        panic("demapPage(Addr) is not implemented.\n");
    }

    void dumpAll();

    // Checkpointing
    virtual void serialize(std::ostream &os);
    virtual void unserialize(Checkpoint *cp, const std::string &section);

    /** Give an entry id, read that tlb entries' tte */
    uint64_t TteRead(int entry);

};

class ITB : public TLB
{
  public:
    typedef SparcITBParams Params;
    ITB(const Params *p) : TLB(p)
    {
        cacheEntry = NULL;
    }

    Fault translate(RequestPtr &req, ThreadContext *tc);
  private:
    void writeSfsr(bool write, ContextType ct,
            bool se, FaultTypes ft, int asi);
    TlbEntry *cacheEntry;
    friend class DTB;
};

class DTB : public TLB
{
    //DTLB specific state
  protected:
    uint64_t sfar;
  public:
    typedef SparcDTBParams Params;
    DTB(const Params *p) : TLB(p)
    {
        sfar = 0;
        cacheEntry[0] = NULL;
        cacheEntry[1] = NULL;
    }

    Fault translate(RequestPtr &req, ThreadContext *tc, bool write);
#if FULL_SYSTEM
    Tick doMmuRegRead(ThreadContext *tc, Packet *pkt);
    Tick doMmuRegWrite(ThreadContext *tc, Packet *pkt);
#endif
    void GetTsbPtr(ThreadContext *tc, Addr addr, int ctx, Addr *ptrs);

    // Checkpointing
    virtual void serialize(std::ostream &os);
    virtual void unserialize(Checkpoint *cp, const std::string &section);

  private:
    void writeSfsr(Addr a, bool write, ContextType ct,
            bool se, FaultTypes ft, int asi);

    uint64_t MakeTsbPtr(TsbPageSize ps, uint64_t tag_access, uint64_t c0_tsb,
        uint64_t c0_config, uint64_t cX_tsb, uint64_t cX_config);


    TlbEntry *cacheEntry[2];
    ASI cacheAsi[2];
};

}

#endif // __ARCH_SPARC_TLB_HH__
