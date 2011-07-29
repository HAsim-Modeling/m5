/*
 * Copyright (c) 2004, 2005
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
 * Authors: Lisa R. Hsu
 */

/**
 * @file
 * Definitions of LRU tag store for a partitioned cache.
 */

#include <string>

#include "mem/cache/base.hh"
#include "base/intmath.hh"
#include "mem/cache/tags/split_lru.hh"
#include "sim/core.hh"

using namespace std;

SplitBlk*
SplitCacheSet::findBlk(Addr tag) const
{
    for (int i = 0; i < assoc; ++i) {
        if (blks[i]->tag == tag && blks[i]->isValid()) {
            return blks[i];
        }
    }
    return 0;
}


void
SplitCacheSet::moveToHead(SplitBlk *blk)
{
    // nothing to do if blk is already head
    if (blks[0] == blk)
        return;

    // write 'next' block into blks[i], moving up from MRU toward LRU
    // until we overwrite the block we moved to head.

    // start by setting up to write 'blk' into blks[0]
    int i = 0;
    SplitBlk *next = blk;

    do {
        assert(i < assoc);
        // swap blks[i] and next
        SplitBlk *tmp = blks[i];
        blks[i] = next;
        next = tmp;
        ++i;
    } while (next != blk);
}


// create and initialize a LRU/MRU cache structure
SplitLRU::SplitLRU(int _numSets, int _blkSize, int _assoc, int _hit_latency, int _part) :
    numSets(_numSets), blkSize(_blkSize), assoc(_assoc), hitLatency(_hit_latency), part(_part)
{
    // Check parameters
    if (blkSize < 4 || !isPowerOf2(blkSize)) {
        fatal("Block size must be at least 4 and a power of 2");
    }
    if (numSets <= 0 || !isPowerOf2(numSets)) {
        fatal("# of sets must be non-zero and a power of 2");
    }
    if (assoc <= 0) {
        fatal("associativity must be greater than zero");
    }
    if (hitLatency <= 0) {
        fatal("access latency must be greater than zero");
    }

    SplitBlk  *blk;
    int i, j, blkIndex;

    blkMask = blkSize - 1;
    setShift = floorLog2(blkSize);
    setMask = numSets - 1;
    tagShift = setShift + floorLog2(numSets);
    warmedUp = false;
    /** @todo Make warmup percentage a parameter. */
    warmupBound = numSets * assoc;

    sets = new SplitCacheSet[numSets];
    blks = new SplitBlk[numSets * assoc];
    // allocate data storage in one big chunk
    dataBlks = new uint8_t[numSets*assoc*blkSize];

    blkIndex = 0;	// index into blks array
    for (i = 0; i < numSets; ++i) {
        sets[i].assoc = assoc;

        sets[i].blks = new SplitBlk*[assoc];

        // link in the data blocks
        for (j = 0; j < assoc; ++j) {
            // locate next cache block
            blk = &blks[blkIndex];
            blk->data = &dataBlks[blkSize*blkIndex];
            ++blkIndex;

            // invalidate new cache block
            blk->status = 0;

            //EGH Fix Me : do we need to initialize blk?

            // Setting the tag to j is just to prevent long chains in the hash
            // table; won't matter because the block is invalid
            blk->tag = j;
            blk->whenReady = 0;
            blk->isTouched = false;
            blk->size = blkSize;
            sets[i].blks[j]=blk;
            blk->set = i;
            blk->part = part;
        }
    }
}

SplitLRU::~SplitLRU()
{
    delete [] dataBlks;
    delete [] blks;
    delete [] sets;
}

void
SplitLRU::regStats(const std::string &name)
{
    BaseTags::regStats(name);

    hits
        .name(name + ".hits")
        .desc("number of hits on this partition")
        .precision(0)
        ;

    misses
        .name(name + ".misses")
        .desc("number of misses in this partition")
        .precision(0)
        ;
}

// probe cache for presence of given block.
bool
SplitLRU::probe(Addr addr) const
{
    //  return(findBlock(Read, addr, asid) != 0);
    Addr tag = extractTag(addr);
    unsigned myset = extractSet(addr);

    SplitBlk *blk = sets[myset].findBlk(tag);

    return (blk != NULL);	// true if in cache
}

SplitBlk*
SplitLRU::findBlock(Addr addr, int &lat)
{
    Addr tag = extractTag(addr);
    unsigned set = extractSet(addr);
    SplitBlk *blk = sets[set].findBlk(tag);
    lat = hitLatency;
    if (blk != NULL) {
        // move this block to head of the MRU list
        sets[set].moveToHead(blk);
        if (blk->whenReady > curTick && blk->whenReady - curTick > hitLatency){
            lat = blk->whenReady - curTick;
        }
        blk->refCount += 1;
        hits++;
    }

    return blk;
}


SplitBlk*
SplitLRU::findBlock(Addr addr) const
{
    Addr tag = extractTag(addr);
    unsigned set = extractSet(addr);
    SplitBlk *blk = sets[set].findBlk(tag);
    return blk;
}

SplitBlk*
SplitLRU::findReplacement(Addr addr, PacketList &writebacks)
{
    unsigned set = extractSet(addr);
    // grab a replacement candidate
    SplitBlk *blk = sets[set].blks[assoc-1];
    sets[set].moveToHead(blk);
    if (blk->isValid()) {
        replacements[0]++;
        totalRefs += blk->refCount;
        ++sampledRefs;
        blk->refCount = 0;
    } else if (!blk->isTouched) {
        tagsInUse++;
        blk->isTouched = true;
        if (!warmedUp && tagsInUse.value() >= warmupBound) {
            warmedUp = true;
            warmupCycle = curTick;
        }
    }

    misses++;

    return blk;
}

void
SplitLRU::invalidateBlk(SplitLRU::BlkType *blk)
{
    if (blk) {
        blk->status = 0;
        blk->isTouched = false;
        tagsInUse--;
    }
}

void
SplitLRU::cleanupRefs()
{
    for (int i = 0; i < numSets*assoc; ++i) {
        if (blks[i].isValid()) {
            totalRefs += blks[i].refCount;
            ++sampledRefs;
        }
    }
}