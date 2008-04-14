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
 * Definitions of LIFO tag store usable in a partitioned cache.
 */

#include <string>

#include "mem/cache/base.hh"
#include "base/intmath.hh"
#include "mem/cache/tags/split_lifo.hh"
#include "sim/core.hh"
#include "base/trace.hh"

using namespace std;

SplitBlk*
LIFOSet::findBlk(Addr tag) const
{
    for (SplitBlk *blk = firstIn; blk != NULL; blk = blk->next) {
        if (blk->tag == tag && blk->isValid()) {
            return blk;
        }
    }
    return NULL;
}

void
LIFOSet::moveToLastIn(SplitBlk *blk)
{
    if (blk == lastIn)
        return;

    if (blk == firstIn) {
        blk->next->prev = NULL;
    } else {
        blk->prev->next = blk->next;
        blk->next->prev = blk->prev;
    }
    blk->next = NULL;
    blk->prev = lastIn;
    lastIn->next = blk;

    lastIn = blk;
}

void
LIFOSet::moveToFirstIn(SplitBlk *blk)
{
    if (blk == firstIn)
        return;

    if (blk == lastIn) {
        blk->prev->next = NULL;
    } else {
        blk->next->prev = blk->prev;
        blk->prev->next = blk->next;
    }

    blk->prev = NULL;
    blk->next = firstIn;
    firstIn->prev = blk;

    firstIn = blk;
}

// create and initialize a LIFO cache structure
SplitLIFO::SplitLIFO(int _blkSize, int _size, int _ways, int _hit_latency, bool two_Queue, int _part) :
    blkSize(_blkSize), size(_size), numBlks(_size/_blkSize), numSets((_size/_ways)/_blkSize), ways(_ways),
    hitLatency(_hit_latency), twoQueue(two_Queue), part(_part)
{
    if (!isPowerOf2(blkSize))
        fatal("cache block size (in bytes) must be a power of 2");
    if (!(hitLatency > 0))
        fatal("access latency in cycles must be at least on cycle");
    if (_ways == 0)
        fatal("if instantiating a splitLIFO, needs non-zero size!");


    SplitBlk  *blk;
    int i, j, blkIndex;

    setShift = floorLog2(blkSize);
    blkMask = blkSize - 1;
    setMask = numSets - 1;
    tagShift = setShift + floorLog2(numSets);

    warmedUp = false;
    /** @todo Make warmup percentage a parameter. */
    warmupBound = size/blkSize;

    // allocate data blocks
    blks = new SplitBlk[numBlks];
    sets = new LIFOSet[numSets];
    dataBlks = new uint8_t[size];

/*
    // these start off point to same blk
    top = &(blks[0]);
    head = top;
*/

    blkIndex = 0;
    for (i=0; i < numSets; ++i) {
        sets[i].ways = ways;
        sets[i].lastIn = &blks[blkIndex];
        sets[i].firstIn = &blks[blkIndex + ways - 1];

        /* 3 cases:  if there is 1 way, if there are 2 ways, or if there are 3+.
           in the case of 1 way, last in and first out point to the same blocks,
           and the next and prev pointers need to be assigned specially.  and so on
        */
        /* deal with the first way */
        blk = &blks[blkIndex];
        blk->prev = &blks[blkIndex + 1];
        blk->next = NULL;
        blk->data = &dataBlks[blkSize*blkIndex];
        blk->size = blkSize;
        blk->part = part;
        blk->set = i;
        ++blkIndex;

        /* if there are "middle" ways, do them here */
        if (ways > 2) {
            for (j=1; j < ways-1; ++j) {
                blk = &blks[blkIndex];
                blk->data = &dataBlks[blkSize*blkIndex];
                blk->prev = &blks[blkIndex+1];
                blk->next = &blks[blkIndex-1];
                blk->data = &(dataBlks[blkSize*blkIndex]);
                blk->size = blkSize;
                blk->part = part;
                blk->set = i;
                ++blkIndex;
            }
        }

        /* do the final way here, depending on whether the final way is the only
           way or not
        */
        if (ways > 1) {
            blk =  &blks[blkIndex];
            blk->prev = NULL;
            blk->next = &blks[blkIndex - 1];
            blk->data = &dataBlks[blkSize*blkIndex];
            blk->size = blkSize;
            blk->part = part;
            blk->set = i;
            ++blkIndex;
        } else {
            blk->prev = NULL;
        }
    }
    assert(blkIndex == numBlks);
}

SplitLIFO::~SplitLIFO()
{
    delete [] blks;
    delete [] sets;
    delete [] dataBlks;
}

void
SplitLIFO::regStats(const std::string &name)
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

    invalidations
        .name(name + ".invalidations")
        .desc("number of invalidations in this partition")
        .precision(0)
        ;
}

// probe cache for presence of given block.
bool
SplitLIFO::probe(Addr addr) const
{
    Addr tag = extractTag(addr);
    unsigned myset = extractSet(addr);

    SplitBlk* blk = sets[myset].findBlk(tag);
    return (blk != NULL);
}

SplitBlk*
SplitLIFO::findBlock(Addr addr, int &lat)
{
    Addr tag = extractTag(addr);
    unsigned set = extractSet(addr);
    SplitBlk *blk = sets[set].findBlk(tag);

    lat = hitLatency;

    if (blk) {
        DPRINTF(Split, "Found LIFO blk %#x in set %d, with tag %#x\n",
                addr, set, tag);
        hits++;

        if (blk->whenReady > curTick && blk->whenReady - curTick > hitLatency)
            lat = blk->whenReady - curTick;
        blk->refCount +=1;

        if (twoQueue) {
            blk->isUsed = true;
            sets[set].moveToFirstIn(blk);
        } else {
            sets[set].moveToLastIn(blk);
        }
    }

    return blk;
}


SplitBlk*
SplitLIFO::findBlock(Addr addr) const
{
    Addr tag = extractTag(addr);
    unsigned set = extractSet(addr);
    SplitBlk *blk = sets[set].findBlk(tag);

    return blk;
}

SplitBlk*
SplitLIFO::findReplacement(Addr addr, PacketList &writebacks)
{
    unsigned set = extractSet(addr);

    SplitBlk *firstIn = sets[set].firstIn;
    SplitBlk *lastIn = sets[set].lastIn;

    SplitBlk *blk;
    if (twoQueue && firstIn->isUsed) {
        blk = firstIn;
        blk->isUsed = false;
        sets[set].moveToLastIn(blk);
    } else {
        int withValue = sets[set].withValue;
        if (withValue == ways) {
            blk = lastIn;
        } else {
            blk = &(sets[set].firstIn[ways - ++withValue]);
        }
    }

    DPRINTF(Split, "just assigned %#x addr into LIFO, replacing %#x status %#x\n",
            addr, regenerateBlkAddr(blk->tag, set), blk->status);
    if (blk->isValid()) {
        replacements[0]++;
        totalRefs += blk->refCount;
        ++sampledRefs;
        blk->refCount = 0;
    } else {
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
SplitLIFO::invalidateBlk(SplitLIFO::BlkType *blk)
{
    if (blk) {
        blk->status = 0;
        blk->isTouched = false;
        tagsInUse--;
        invalidations++;
    }
}

void
SplitLIFO::cleanupRefs()
{
    for (int i = 0; i < numBlks; ++i) {
        if (blks[i].isValid()) {
            totalRefs += blks[i].refCount;
            ++sampledRefs;
        }
    }
}
