/*
 * Copyright  2003, 2004, 2005
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
 *          Nathan L. Binkert
 */

/**
 * @file
 * Simobject instatiation of caches.
 */
#include <vector>

// Must be included first to determine which caches we want
#include "enums/Prefetch.hh"
#include "mem/config/cache.hh"
#include "mem/config/prefetch.hh"
#include "mem/cache/base.hh"
#include "mem/cache/cache.hh"
#include "mem/bus.hh"
#include "params/BaseCache.hh"

// Tag Templates
#if defined(USE_CACHE_LRU)
#include "mem/cache/tags/lru.hh"
#endif

#if defined(USE_CACHE_FALRU)
#include "mem/cache/tags/fa_lru.hh"
#endif

#if defined(USE_CACHE_IIC)
#include "mem/cache/tags/iic.hh"
#endif

#if defined(USE_CACHE_SPLIT)
#include "mem/cache/tags/split.hh"
#endif

#if defined(USE_CACHE_SPLIT_LIFO)
#include "mem/cache/tags/split_lifo.hh"
#endif

//Prefetcher Headers
#if defined(USE_GHB)
#include "mem/cache/prefetch/ghb.hh"
#endif
#if defined(USE_TAGGED)
#include "mem/cache/prefetch/tagged.hh"
#endif
#if defined(USE_STRIDED)
#include "mem/cache/prefetch/stride.hh"
#endif


using namespace std;
using namespace TheISA;

#define BUILD_CACHE(TAGS, tags)                                      \
    do {                                                                \
        BasePrefetcher *pf;                                             \
        if (prefetch_policy == Enums::tagged) {                         \
            BUILD_TAGGED_PREFETCHER(TAGS);                              \
        }                                                               \
        else if (prefetch_policy == Enums::stride) {                    \
            BUILD_STRIDED_PREFETCHER(TAGS);                             \
        }                                                               \
        else if (prefetch_policy == Enums::ghb) {                       \
            BUILD_GHB_PREFETCHER(TAGS);                                 \
        }                                                               \
        else {                                                          \
            BUILD_NULL_PREFETCHER(TAGS);                                \
        }                                                               \
        Cache<TAGS> *retval =                                           \
            new Cache<TAGS>(this, tags, pf);                            \
        return retval;                                                  \
    } while (0)

#define BUILD_CACHE_PANIC(x) do {			\
        panic("%s not compiled into M5", x);		\
    } while (0)

#if defined(USE_CACHE_FALRU)
#define BUILD_FALRU_CACHE do {			    \
        FALRU *tags = new FALRU(block_size, size, latency); \
        BUILD_CACHE(FALRU, tags);		\
    } while (0)
#else
#define BUILD_FALRU_CACHE BUILD_CACHE_PANIC("falru cache")
#endif

#if defined(USE_CACHE_LRU)
#define BUILD_LRU_CACHE do {				\
        LRU *tags = new LRU(numSets, block_size, assoc, latency);	\
        BUILD_CACHE(LRU, tags);			\
    } while (0)
#else
#define BUILD_LRU_CACHE BUILD_CACHE_PANIC("lru cache")
#endif

#if defined(USE_CACHE_SPLIT)
#define BUILD_SPLIT_CACHE do {					\
        Split *tags = new Split(numSets, block_size, assoc, split_size, lifo, \
                                two_queue, latency);		\
        BUILD_CACHE(Split, tags);			\
    } while (0)
#else
#define BUILD_SPLIT_CACHE BUILD_CACHE_PANIC("split cache")
#endif

#if defined(USE_CACHE_SPLIT_LIFO)
#define BUILD_SPLIT_LIFO_CACHE do {				\
        SplitLIFO *tags = new SplitLIFO(block_size, size, assoc,        \
                                        latency, two_queue, -1);	\
        BUILD_CACHE(SplitLIFO, tags);			\
    } while (0)
#else
#define BUILD_SPLIT_LIFO_CACHE BUILD_CACHE_PANIC("lifo cache")
#endif

#if defined(USE_CACHE_IIC)
#define BUILD_IIC_CACHE do {			\
        IIC *tags = new IIC(iic_params);		\
        BUILD_CACHE(IIC, tags);	\
    } while (0)
#else
#define BUILD_IIC_CACHE BUILD_CACHE_PANIC("iic")
#endif

#define BUILD_CACHES do {				\
        if (repl == NULL) {				\
            if (numSets == 1) {				\
                BUILD_FALRU_CACHE;		\
            } else {					\
                if (split == true) {			\
                    BUILD_SPLIT_CACHE;		\
                } else if (lifo == true) {		\
                    BUILD_SPLIT_LIFO_CACHE;	\
                } else {				\
                    BUILD_LRU_CACHE;		\
                }					\
            }						\
        } else {					\
            BUILD_IIC_CACHE;			\
        }						\
    } while (0)

#define BUILD_COHERENCE(b) do {						\
    } while (0)

#if defined(USE_TAGGED)
#define BUILD_TAGGED_PREFETCHER(t)                              \
    pf = new TaggedPrefetcher(this)
#else
#define BUILD_TAGGED_PREFETCHER(t) BUILD_CACHE_PANIC("Tagged Prefetcher")
#endif

#if defined(USE_STRIDED)
#define BUILD_STRIDED_PREFETCHER(t)                             \
    pf = new StridePrefetcher(this)
#else
#define BUILD_STRIDED_PREFETCHER(t) BUILD_CACHE_PANIC("Stride Prefetcher")
#endif

#if defined(USE_GHB)
#define BUILD_GHB_PREFETCHER(t)                         \
    pf = new GHBPrefetcher(this)
#else
#define BUILD_GHB_PREFETCHER(t) BUILD_CACHE_PANIC("GHB Prefetcher")
#endif

#if defined(USE_TAGGED)
#define BUILD_NULL_PREFETCHER(t)                                \
    pf = new TaggedPrefetcher(this)
#else
#define BUILD_NULL_PREFETCHER(t) BUILD_CACHE_PANIC("NULL Prefetcher (uses Tagged)")
#endif

BaseCache *
BaseCacheParams::create()
{
    int numSets = size / (assoc * block_size);
    if (subblock_size == 0) {
        subblock_size = block_size;
    }

    //Warnings about prefetcher policy
    if (prefetch_policy == Enums::none) {
        if (prefetch_miss || prefetch_access)
            panic("With no prefetcher, you shouldn't prefetch from"
                  " either miss or access stream\n");
    }

    if (prefetch_policy == Enums::tagged || prefetch_policy == Enums::stride ||
        prefetch_policy == Enums::ghb) {

        if (!prefetch_miss && !prefetch_access)
            warn("With this prefetcher you should chose a prefetch"
                 " stream (miss or access)\nNo Prefetching will occur\n");

        if (prefetch_miss && prefetch_access)
            panic("Can't do prefetches from both miss and access stream");
    }

#if defined(USE_CACHE_IIC)
    // Build IIC params
    IIC::Params iic_params;
    iic_params.size = size;
    iic_params.numSets = numSets;
    iic_params.blkSize = block_size;
    iic_params.assoc = assoc;
    iic_params.hashDelay = hash_delay;
    iic_params.hitLatency = latency;
    iic_params.rp = repl;
    iic_params.subblockSize = subblock_size;
#else
    const void *repl = NULL;
#endif

    BUILD_CACHES;
    return NULL;
}
