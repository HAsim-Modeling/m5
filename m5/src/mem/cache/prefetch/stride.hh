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
 * Describes a strided prefetcher.
 */

#ifndef __MEM_CACHE_PREFETCH_STRIDE_PREFETCHER_HH__
#define __MEM_CACHE_PREFETCH_STRIDE_PREFETCHER_HH__

#include "mem/cache/prefetch/base.hh"

class StridePrefetcher : public BasePrefetcher
{
  protected:

    class strideEntry
    {
      public:
        Addr IAddr;
        Addr MAddr;
        int stride;
        int64_t confidence;

/*	bool operator < (strideEntry a,strideEntry b)
        {
            if (a.confidence == b.confidence) {
                return true; //??????
            }
            else return a.confidence < b.confidence;
            }*/
    };
    Addr* lastMissAddr[64/*MAX_CPUS*/];

    std::list<strideEntry*> table[64/*MAX_CPUS*/];
    Tick latency;
    int degree;
    bool useCPUId;


  public:

    StridePrefetcher(const BaseCacheParams *p)
        : BasePrefetcher(p), latency(p->prefetch_latency),
          degree(p->prefetch_degree), useCPUId(p->prefetch_use_cpu_id)
    {
    }

    ~StridePrefetcher() {}

    void calculatePrefetch(PacketPtr &pkt, std::list<Addr> &addresses,
                           std::list<Tick> &delays);
};

#endif // __MEM_CACHE_PREFETCH_STRIDE_PREFETCHER_HH__
