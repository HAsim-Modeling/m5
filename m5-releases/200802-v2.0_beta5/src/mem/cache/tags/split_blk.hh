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
 * Declaration of partitioned tag store cache block class.
 */

#ifndef __SPLIT_BLK_HH__
#define __SPLIT_BLK_HH__

#include "mem/cache/blk.hh" // base class

/**
 * Split cache block.
 */
class SplitBlk : public CacheBlk {
  public:
    /** Has this block been touched? Used to aid calculation of warmup time. */
    bool isTouched;
    /** Has this block been used after being brought in? (for LIFO partition) */
    bool isUsed;
    /** is this blk a NIC block? (i.e. requested by the NIC) */
    bool isNIC;
    /** timestamp of the arrival of this block into the cache */
    Tick ts;
    /** the previous block in the LIFO partition (brought in before than me) */
    SplitBlk *prev;
    /** the next block in the LIFO partition (brought in later than me) */
    SplitBlk *next;
    /** which partition this block is in */
    int part;

    SplitBlk()
        : isTouched(false), isUsed(false), isNIC(false), ts(0), prev(NULL), next(NULL),
          part(0)
    {}
};

#endif

