/*
 * Copyright (c) 2003, 2004, 2005
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
 */

/** @file
 * Definitions of a simple cache block class.
 */

#ifndef __CACHE_BLK_HH__
#define __CACHE_BLK_HH__

#include <list>

#include "base/printable.hh"
#include "sim/core.hh"		// for Tick
#include "arch/isa_traits.hh"	// for Addr
#include "mem/packet.hh"
#include "mem/request.hh"

/**
 * Cache block status bit assignments
 */
enum CacheBlkStatusBits {
    /** valid, readable */
    BlkValid =		0x01,
    /** write permission */
    BlkWritable =	0x02,
    /** read permission (yes, block can be valid but not readable) */
    BlkReadable =	0x04,
    /** dirty (modified) */
    BlkDirty =		0x08,
    /** block was referenced */
    BlkReferenced =	0x10,
    /** block was a hardware prefetch yet unaccessed*/
    BlkHWPrefetched =	0x20
};

/**
 * A Basic Cache block.
 * Contains the tag, status, and a pointer to data.
 */
class CacheBlk
{
  public:
    /** The address space ID of this block. */
    int asid;
    /** Data block tag value. */
    Addr tag;
    /**
     * Contains a copy of the data in this block for easy access. This is used
     * for efficient execution when the data could be actually stored in
     * another format (COW, compressed, sub-blocked, etc). In all cases the
     * data stored here should be kept consistant with the actual data
     * referenced by this block.
     */
    uint8_t *data;
    /** the number of bytes stored in this block. */
    int size;

    /** block state: OR of CacheBlkStatusBit */
    typedef unsigned State;

    /** The current status of this block. @sa CacheBlockStatusBits */
    State status;

    /** Which curTick will this block be accessable */
    Tick whenReady;

    /**
     * The set this block belongs to.
     * @todo Move this into subclasses when we fix CacheTags to use them.
     */
    int set;

    /** Number of references to this block since it was brought in. */
    int refCount;

  protected:
    /**
     * Represents that the indicated thread context has a "lock" on
     * the block, in the LL/SC sense.
     */
    class Lock {
      public:
        int cpuNum;	// locking CPU
        int threadNum;	// locking thread ID within CPU

        // check for matching execution context
        bool matchesContext(Request *req)
        {
            return (cpuNum == req->getCpuNum() &&
                    threadNum == req->getThreadNum());
        }

        Lock(Request *req)
            : cpuNum(req->getCpuNum()), threadNum(req->getThreadNum())
        {
        }
    };

    /** List of thread contexts that have performed a load-locked (LL)
     * on the block since the last store. */
    std::list<Lock> lockList;

  public:

    CacheBlk()
        : asid(-1), tag(0), data(0) ,size(0), status(0), whenReady(0),
          set(-1), refCount(0)
    {}

    /**
     * Copy the state of the given block into this one.
     * @param rhs The block to copy.
     * @return a const reference to this block.
     */
    const CacheBlk& operator=(const CacheBlk& rhs)
    {
        asid = rhs.asid;
        tag = rhs.tag;
        data = rhs.data;
        size = rhs.size;
        status = rhs.status;
        whenReady = rhs.whenReady;
        set = rhs.set;
        refCount = rhs.refCount;
        return *this;
    }

    /**
     * Checks the write permissions of this block.
     * @return True if the block is writable.
     */
    bool isWritable() const
    {
        const int needed_bits = BlkWritable | BlkValid;
        return (status & needed_bits) == needed_bits;
    }

    /**
     * Checks the read permissions of this block.  Note that a block
     * can be valid but not readable if there is an outstanding write
     * upgrade miss.
     * @return True if the block is readable.
     */
    bool isReadable() const
    {
        const int needed_bits = BlkReadable | BlkValid;
        return (status & needed_bits) == needed_bits;
    }

    /**
     * Checks that a block is valid.
     * @return True if the block is valid.
     */
    bool isValid() const
    {
        return (status & BlkValid) != 0;
    }

    /**
     * Check to see if a block has been written.
     * @return True if the block is dirty.
     */
    bool isDirty() const
    {
        return (status & BlkDirty) != 0;
    }

    /**
     * Check if this block has been referenced.
     * @return True if the block has been referenced.
     */
    bool isReferenced() const
    {
        return (status & BlkReferenced) != 0;
    }

    /**
     * Check if this block was the result of a hardware prefetch, yet to
     * be touched.
     * @return True if the block was a hardware prefetch, unaccesed.
     */
    bool isPrefetch() const
    {
        return (status & BlkHWPrefetched) != 0;
    }

    /**
     * Track the fact that a local locked was issued to the block.  If
     * multiple LLs get issued from the same context we could have
     * redundant records on the list, but that's OK, as they'll all
     * get blown away at the next store.
     */
    void trackLoadLocked(PacketPtr pkt)
    {
        assert(pkt->isLocked());
        lockList.push_front(Lock(pkt->req));
    }

    /**
     * Clear the list of valid load locks.  Should be called whenever
     * block is written to or invalidated.
     */
    void clearLoadLocks() { lockList.clear(); }

    /**
     * Handle interaction of load-locked operations and stores.
     * @return True if write should proceed, false otherwise.  Returns
     * false only in the case of a failed store conditional.
     */
    bool checkWrite(PacketPtr pkt)
    {
        Request *req = pkt->req;
        if (pkt->isLocked()) {
            // it's a store conditional... have to check for matching
            // load locked.
            bool success = false;

            for (std::list<Lock>::iterator i = lockList.begin();
                 i != lockList.end(); ++i)
            {
                if (i->matchesContext(req)) {
                    // it's a store conditional, and as far as the memory
                    // system can tell, the requesting context's lock is
                    // still valid.
                    success = true;
                    break;
                }
            }

            req->setExtraData(success ? 1 : 0);
            clearLoadLocks();
            return success;
        } else {
            // for *all* stores (conditional or otherwise) we have to
            // clear the list of load-locks as they're all invalid now.
            clearLoadLocks();
            return true;
        }
    }
};

/**
 * Simple class to provide virtual print() method on cache blocks
 * without allocating a vtable pointer for every single cache block.
 * Just wrap the CacheBlk object in an instance of this before passing
 * to a function that requires a Printable object.
 */
class CacheBlkPrintWrapper : public Printable
{
    CacheBlk *blk;
  public:
    CacheBlkPrintWrapper(CacheBlk *_blk) : blk(_blk) {}
    virtual ~CacheBlkPrintWrapper() {}
    void print(std::ostream &o, int verbosity = 0,
               const std::string &prefix = "") const;
};



#endif //__CACHE_BLK_HH__
