/*
 * Copyright (c) 2005
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
 * Authors: Ronald G. Dreslinski Jr
 */

/**
 * @file
 * Hardware Prefetcher Definition.
 */

#include "base/trace.hh"
#include "mem/cache/base.hh"
#include "mem/cache/prefetch/base.hh"
#include "mem/request.hh"
#include <list>

BasePrefetcher::BasePrefetcher(const BaseCacheParams *p)
    : size(p->prefetcher_size), pageStop(!p->prefetch_past_page),
      serialSquash(p->prefetch_serial_squash),
      cacheCheckPush(p->prefetch_cache_check_push),
      only_data(p->prefetch_data_accesses_only)
{
}

void
BasePrefetcher::setCache(BaseCache *_cache)
{
    cache = _cache;
    blkSize = cache->getBlockSize();
}

void
BasePrefetcher::regStats(const std::string &name)
{
    pfIdentified
        .name(name + ".prefetcher.num_hwpf_identified")
        .desc("number of hwpf identified")
        ;

    pfMSHRHit
        .name(name + ".prefetcher.num_hwpf_already_in_mshr")
        .desc("number of hwpf that were already in mshr")
        ;

    pfCacheHit
        .name(name + ".prefetcher.num_hwpf_already_in_cache")
        .desc("number of hwpf that were already in the cache")
        ;

    pfBufferHit
        .name(name + ".prefetcher.num_hwpf_already_in_prefetcher")
        .desc("number of hwpf that were already in the prefetch queue")
        ;

    pfRemovedFull
        .name(name + ".prefetcher.num_hwpf_evicted")
        .desc("number of hwpf removed due to no buffer left")
        ;

    pfRemovedMSHR
        .name(name + ".prefetcher.num_hwpf_removed_MSHR_hit")
        .desc("number of hwpf removed because MSHR allocated")
        ;

    pfIssued
        .name(name + ".prefetcher.num_hwpf_issued")
        .desc("number of hwpf issued")
        ;

    pfSpanPage
        .name(name + ".prefetcher.num_hwpf_span_page")
        .desc("number of hwpf spanning a virtual page")
        ;

    pfSquashed
        .name(name + ".prefetcher.num_hwpf_squashed_from_miss")
        .desc("number of hwpf that got squashed due to a miss aborting calculation time")
        ;
}

inline bool
BasePrefetcher::inCache(Addr addr)
{
    if (cache->inCache(addr)) {
        pfCacheHit++;
        return true;
    }
    return false;
}

inline bool
BasePrefetcher::inMissQueue(Addr addr)
{
    if (cache->inMissQueue(addr)) {
        pfMSHRHit++;
        return true;
    }
    return false;
}

PacketPtr
BasePrefetcher::getPacket()
{
    DPRINTF(HWPrefetch, "%s:Requesting a hw_pf to issue\n", cache->name());

    if (pf.empty()) {
        DPRINTF(HWPrefetch, "%s:No HW_PF found\n", cache->name());
        return NULL;
    }

    PacketPtr pkt;
    bool keepTrying = false;
    do {
        pkt = *pf.begin();
        pf.pop_front();
        if (!cacheCheckPush) {
            keepTrying = cache->inCache(pkt->getAddr());
        }
        if (pf.empty()) {
            cache->deassertMemSideBusRequest(BaseCache::Request_PF);
            if (keepTrying) return NULL; //None left, all were in cache
        }
    } while (keepTrying);

    pfIssued++;
    return pkt;
}

void
BasePrefetcher::handleMiss(PacketPtr &pkt, Tick time)
{
    if (!pkt->req->isUncacheable() && !(pkt->req->isInstRead() && only_data))
    {
        //Calculate the blk address
        Addr blkAddr = pkt->getAddr() & ~(Addr)(blkSize-1);

        //Check if miss is in pfq, if so remove it
        std::list<PacketPtr>::iterator iter = inPrefetch(blkAddr);
        if (iter != pf.end()) {
            DPRINTF(HWPrefetch, "%s:Saw a miss to a queued prefetch, removing it\n", cache->name());
            pfRemovedMSHR++;
            pf.erase(iter);
            if (pf.empty())
                cache->deassertMemSideBusRequest(BaseCache::Request_PF);
        }

        //Remove anything in queue with delay older than time
        //since everything is inserted in time order, start from end
        //and work until pf.empty() or time is earlier
        //This is done to emulate Aborting the previous work on a new miss
        //Needed for serial calculators like GHB
        if (serialSquash) {
            iter = pf.end();
            iter--;
            while (!pf.empty() && ((*iter)->time >= time)) {
                pfSquashed++;
                pf.pop_back();
                iter--;
            }
            if (pf.empty())
                cache->deassertMemSideBusRequest(BaseCache::Request_PF);
        }


        std::list<Addr> addresses;
        std::list<Tick> delays;
        calculatePrefetch(pkt, addresses, delays);

        std::list<Addr>::iterator addr = addresses.begin();
        std::list<Tick>::iterator delay = delays.begin();
        while (addr != addresses.end())
        {
            DPRINTF(HWPrefetch, "%s:Found a pf canidate, inserting into prefetch queue\n", cache->name());
            //temp calc this here...
            pfIdentified++;
            //create a prefetch memreq
            Request * prefetchReq = new Request(*addr, blkSize, 0);
            PacketPtr prefetch;
            prefetch = new Packet(prefetchReq, MemCmd::HardPFReq, -1);
            prefetch->allocate();
            prefetch->req->setThreadContext(pkt->req->getCpuNum(),
                                            pkt->req->getThreadNum());

            prefetch->time = time + (*delay); //@todo ADD LATENCY HERE
            //... initialize

            //Check if it is already in the cache
            if (cacheCheckPush) {
                if (cache->inCache(prefetch->getAddr())) {
                    addr++;
                    delay++;
                    continue;
                }
            }

            //Check if it is already in the miss_queue
            if (cache->inMissQueue(prefetch->getAddr())) {
                addr++;
                delay++;
                continue;
            }

            //Check if it is already in the pf buffer
            if (inPrefetch(prefetch->getAddr()) != pf.end()) {
                pfBufferHit++;
                addr++;
                delay++;
                continue;
            }

            //We just remove the head if we are full
            if (pf.size() == size)
            {
                DPRINTF(HWPrefetch, "%s:Inserting into prefetch queue, it was full removing oldest\n", cache->name());
                pfRemovedFull++;
                pf.pop_front();
            }

            pf.push_back(prefetch);

            //Make sure to request the bus, with proper delay
            cache->requestMemSideBus(BaseCache::Request_PF, prefetch->time);

            //Increment through the list
            addr++;
            delay++;
        }
    }
}

std::list<PacketPtr>::iterator
BasePrefetcher::inPrefetch(Addr address)
{
    //Guaranteed to only be one match, we always check before inserting
    std::list<PacketPtr>::iterator iter;
    for (iter=pf.begin(); iter != pf.end(); iter++) {
        if (((*iter)->getAddr() & ~(Addr)(blkSize-1)) == address) {
            return iter;
        }
    }
    return pf.end();
}


