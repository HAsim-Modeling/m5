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
 * Authors: Steven K. Reinhardt
 */

#ifndef __BASE__CHUNK_GENERATOR_HH__
#define __BASE__CHUNK_GENERATOR_HH__

/**
 * @file
 * Declaration and inline definition of ChunkGenerator object.
 */

#include <algorithm>
#include "base/intmath.hh"
#include "arch/isa_traits.hh" // for Addr

/**
 * This class takes an arbitrary memory region (address/length pair)
 * and generates a series of appropriately (e.g. block- or page-)
 * aligned chunks covering the same region.
 *
 * Example usage:

\code
    for (ChunkGenerator gen(addr, size, chunkSize); !gen.done(); gen.next()) {
        doSomethingChunky(gen.addr(), gen.size());
    }
\endcode
 */
class ChunkGenerator
{
  private:
    /** The starting address of the current chunk. */
    Addr curAddr;
    /** The starting address of the next chunk (after the current one). */
    Addr nextAddr;
    /** The size of the current chunk (in bytes). */
    int  curSize;
    /** The number of bytes remaining in the region after the current chunk. */
    int  sizeLeft;
    /** The start address so we can calculate offset in writing block. */
    const Addr startAddr;
    /** The maximum chunk size, e.g., the cache block size or page size. */
    const int chunkSize;

  public:
    /**
     * Constructor.
     * @param _startAddr The starting address of the region.
     * @param totalSize The total size of the region.
     * @param _chunkSize The size/alignment of chunks into which
     *    the region should be decomposed.
     */
    ChunkGenerator(Addr _startAddr, int totalSize, int _chunkSize)
        : startAddr(_startAddr), chunkSize(_chunkSize)
    {
        // chunkSize must be a power of two
        assert(chunkSize == 0 || isPowerOf2(chunkSize));

        // set up initial chunk.
        curAddr = startAddr;

        if (chunkSize == 0) //Special Case, if we see 0, assume no chuncking
        {
            nextAddr = startAddr + totalSize;
        }
        else
        {
            // nextAddr should be *next* chunk start
            nextAddr = roundUp(startAddr, chunkSize);
            if (curAddr == nextAddr) {
                // ... even if startAddr is already chunk-aligned
                nextAddr += chunkSize;
            }
        }

        // how many bytes are left between curAddr and the end of this chunk?
        int left_in_chunk = nextAddr - curAddr;
        curSize = std::min(totalSize, left_in_chunk);
        sizeLeft = totalSize - curSize;
    }

    /** Return starting address of current chunk. */
    Addr addr() { return curAddr; }
    /** Return size in bytes of current chunk. */
    int  size() { return curSize; }

    /** Number of bytes we have already chunked up. */
    int complete() { return curAddr - startAddr; }
    /**
     * Are we done?  That is, did the last call to next() advance
     * past the end of the region?
     * @return True if yes, false if more to go.
     */
    bool done() { return (curSize == 0); }

    /**
     * Advance generator to next chunk.
     * @return True if successful, false if unsuccessful
     * (because we were at the last chunk).
     */
    bool next()
    {
        if (sizeLeft == 0) {
            curSize = 0;
            return false;
        }

        curAddr = nextAddr;
        curSize = std::min(sizeLeft, chunkSize);
        sizeLeft -= curSize;
        nextAddr += curSize;
        return true;
    }
};

#endif // __BASE__CHUNK_GENERATOR_HH__
