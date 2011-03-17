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
 * Authors: Erik G. Hallnor
 *          David A. Greene
 *          Steven K. Reinhardt
 *          Ronald G. Dreslinski
 */

/**
 * @file
 * Describes a cache based on template policies.
 */

#ifndef __CACHE_HH__
#define __CACHE_HH__

#include "base/misc.hh" // fatal, panic, and warn

#include "mem/cache/base.hh"
#include "mem/cache/blk.hh"
#include "mem/cache/mshr.hh"

#include "sim/eventq.hh"

//Forward decleration
class BasePrefetcher;

/**
 * A template-policy based cache. The behavior of the cache can be altered by
 * supplying different template policies. TagStore handles all tag and data
 * storage @sa TagStore.
 */
template <class TagStore>
class Cache : public BaseCache
{
  public:
    /** Define the type of cache block to use. */
    typedef typename TagStore::BlkType BlkType;
    /** A typedef for a list of BlkType pointers. */
    typedef typename TagStore::BlkList BlkList;

    bool prefetchAccess;

  protected:

    class CpuSidePort : public CachePort
    {
      public:
        CpuSidePort(const std::string &_name,
                    Cache<TagStore> *_cache,
                    const std::string &_label,
                    std::vector<Range<Addr> > filterRanges);

        // BaseCache::CachePort just has a BaseCache *; this function
        // lets us get back the type info we lost when we stored the
        // cache pointer there.
        Cache<TagStore> *myCache() {
            return static_cast<Cache<TagStore> *>(cache);
        }

        virtual void getDeviceAddressRanges(AddrRangeList &resp,
                                            bool &snoop);

        virtual bool recvTiming(PacketPtr pkt);

        virtual Tick recvAtomic(PacketPtr pkt);

        virtual void recvFunctional(PacketPtr pkt);
    };

    class MemSidePort : public CachePort
    {
      public:
        MemSidePort(const std::string &_name,
                    Cache<TagStore> *_cache,
                    const std::string &_label,
                    std::vector<Range<Addr> > filterRanges);

        // BaseCache::CachePort just has a BaseCache *; this function
        // lets us get back the type info we lost when we stored the
        // cache pointer there.
        Cache<TagStore> *myCache() {
            return static_cast<Cache<TagStore> *>(cache);
        }

        void sendPacket();

        void processSendEvent();

        virtual void getDeviceAddressRanges(AddrRangeList &resp,
                                            bool &snoop);

        virtual bool recvTiming(PacketPtr pkt);

        virtual void recvRetry();

        virtual Tick recvAtomic(PacketPtr pkt);

        virtual void recvFunctional(PacketPtr pkt);

        typedef EventWrapper<MemSidePort, &MemSidePort::processSendEvent>
                SendEvent;
    };

    /** Tag and data Storage */
    TagStore *tags;

    /** Prefetcher */
    BasePrefetcher *prefetcher;

    /** Temporary cache block for occasional transitory use */
    BlkType *tempBlock;

    /**
     * Can this cache should allocate a block on a line-sized write miss.
     */
    const bool doFastWrites;

    const bool prefetchMiss;

    /**
     * Handle a replacement for the given request.
     * @param blk A pointer to the block, usually NULL
     * @param pkt The memory request to satisfy.
     * @param new_state The new state of the block.
     * @param writebacks A list to store any generated writebacks.
     */
    BlkType* doReplacement(BlkType *blk, PacketPtr pkt,
                           CacheBlk::State new_state, PacketList &writebacks);

    /**
     * Does all the processing necessary to perform the provided request.
     * @param pkt The memory request to perform.
     * @param lat The latency of the access.
     * @param writebacks List for any writebacks that need to be performed.
     * @param update True if the replacement data should be updated.
     * @return Pointer to the cache block touched by the request. NULL if it
     * was a miss.
     */
    bool access(PacketPtr pkt, BlkType *&blk, int &lat);

    /**
     *Handle doing the Compare and Swap function for SPARC.
     */
    void cmpAndSwap(BlkType *blk, PacketPtr pkt);

    /**
     * Find a block frame for new block at address addr, assuming that
     * the block is not currently in the cache.  Append writebacks if
     * any to provided packet list.  Return free block frame.  May
     * return NULL if there are no replaceable blocks at the moment.
     */
    BlkType *allocateBlock(Addr addr, PacketList &writebacks);

    /**
     * Populates a cache block and handles all outstanding requests for the
     * satisfied fill request. This version takes two memory requests. One
     * contains the fill data, the other is an optional target to satisfy.
     * Used for Cache::probe.
     * @param pkt The memory request with the fill data.
     * @param blk The cache block if it already exists.
     * @param writebacks List for any writebacks that need to be performed.
     * @return Pointer to the new cache block.
     */
    BlkType *handleFill(PacketPtr pkt, BlkType *blk,
                        PacketList &writebacks);

    void satisfyCpuSideRequest(PacketPtr pkt, BlkType *blk);
    bool satisfyMSHR(MSHR *mshr, PacketPtr pkt, BlkType *blk);

    void doTimingSupplyResponse(PacketPtr req_pkt, uint8_t *blk_data,
                                bool already_copied, bool pending_inval);

    /**
     * Sets the blk to the new state.
     * @param blk The cache block being snooped.
     * @param new_state The new coherence state for the block.
     */
    void handleSnoop(PacketPtr ptk, BlkType *blk,
                     bool is_timing, bool is_deferred, bool pending_inval);

    /**
     * Create a writeback request for the given block.
     * @param blk The block to writeback.
     * @return The writeback request for the block.
     */
    PacketPtr writebackBlk(BlkType *blk);

  public:
    /** Instantiates a basic cache object. */
    Cache(const Params *p, TagStore *tags, BasePrefetcher *prefetcher);

    virtual Port *getPort(const std::string &if_name, int idx = -1);
    virtual void deletePortRefs(Port *p);

    void regStats();

    /**
     * Performs the access specified by the request.
     * @param pkt The request to perform.
     * @return The result of the access.
     */
    bool timingAccess(PacketPtr pkt);

    /**
     * Performs the access specified by the request.
     * @param pkt The request to perform.
     * @return The result of the access.
     */
    Tick atomicAccess(PacketPtr pkt);

    /**
     * Performs the access specified by the request.
     * @param pkt The request to perform.
     * @return The result of the access.
     */
    void functionalAccess(PacketPtr pkt, CachePort *incomingPort,
                          CachePort *otherSidePort);

    /**
     * Handles a response (cache line fill/write ack) from the bus.
     * @param pkt The request being responded to.
     */
    void handleResponse(PacketPtr pkt);

    /**
     * Snoops bus transactions to maintain coherence.
     * @param pkt The current bus transaction.
     */
    void snoopTiming(PacketPtr pkt);

    /**
     * Snoop for the provided request in the cache and return the estimated
     * time of completion.
     * @param pkt The memory request to snoop
     * @return The estimated completion time.
     */
    Tick snoopAtomic(PacketPtr pkt);

    /**
     * Squash all requests associated with specified thread.
     * intended for use by I-cache.
     * @param threadNum The thread to squash.
     */
    void squash(int threadNum);

    /**
     * Generate an appropriate downstream bus request packet for the
     * given parameters.
     * @param cpu_pkt  The upstream request that needs to be satisfied.
     * @param blk The block currently in the cache corresponding to
     * cpu_pkt (NULL if none).
     * @param needsExclusive  Indicates that an exclusive copy is required
     * even if the request in cpu_pkt doesn't indicate that.
     * @return A new Packet containing the request, or NULL if the
     * current request in cpu_pkt should just be forwarded on.
     */
    PacketPtr getBusPacket(PacketPtr cpu_pkt, BlkType *blk,
                           bool needsExclusive);

    /**
     * Return the next MSHR to service, either a pending miss from the
     * mshrQueue, a buffered write from the write buffer, or something
     * from the prefetcher.  This function is responsible for
     * prioritizing among those sources on the fly.
     */
    MSHR *getNextMSHR();

    /**
     * Selects an outstanding request to service.  Called when the
     * cache gets granted the downstream bus in timing mode.
     * @return The request to service, NULL if none found.
     */
    PacketPtr getTimingPacket();

    /**
     * Marks a request as in service (sent on the bus). This can have side
     * effect since storage for no response commands is deallocated once they
     * are successfully sent.
     * @param pkt The request that was sent on the bus.
     */
    void markInService(MSHR *mshr);

    /**
     * Perform the given writeback request.
     * @param pkt The writeback request.
     */
    void doWriteback(PacketPtr pkt);

    /**
     * Return whether there are any outstanding misses.
     */
    bool outstandingMisses() const
    {
        return mshrQueue.allocated != 0;
    }

    CacheBlk *findBlock(Addr addr) {
        return tags->findBlock(addr);
    }

    bool inCache(Addr addr) {
        return (tags->findBlock(addr) != 0);
    }

    bool inMissQueue(Addr addr) {
        return (mshrQueue.findMatch(addr) != 0);
    }
};

#endif // __CACHE_HH__
