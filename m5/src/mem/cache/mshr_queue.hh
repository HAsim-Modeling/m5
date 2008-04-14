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
 * Declaration of a structure to manage MSHRs.
 */

#ifndef __MEM__CACHE__MISS__MSHR_QUEUE_HH__
#define __MEM__CACHE__MISS__MSHR_QUEUE_HH__

#include <vector>

#include "mem/packet.hh"
#include "mem/cache/mshr.hh"

/**
 * A Class for maintaining a list of pending and allocated memory requests.
 */
class MSHRQueue
{
  private:
    /** Local label (for functional print requests) */
    const std::string label;

    /**  MSHR storage. */
    MSHR *registers;
    /** Holds pointers to all allocated entries. */
    MSHR::List allocatedList;
    /** Holds pointers to entries that haven't been sent to the bus. */
    MSHR::List readyList;
    /** Holds non allocated entries. */
    MSHR::List freeList;

    // Parameters
    /**
     * The total number of entries in this queue. This number is set as the
     * number of entries requested plus (numReserve - 1). This allows for
     * the same number of effective entries while still maintaining the reserve.
     */
    const int numEntries;

    /**
     * The number of entries to hold in reserve. This is needed because copy
     * operations can allocate upto 4 entries at one time.
     */
    const int numReserve;

    MSHR::Iterator addToReadyList(MSHR *mshr);


  public:
    /** The number of allocated entries. */
    int allocated;
    /** The number of entries that have been forwarded to the bus. */
    int inServiceEntries;
    /** The index of this queue within the cache (MSHR queue vs. write
     * buffer). */
    const int index;

    /**
     * Create a queue with a given number of entries.
     * @param num_entrys The number of entries in this queue.
     * @param reserve The minimum number of entries needed to satisfy
     * any access.
     */
    MSHRQueue(const std::string &_label, int num_entries, int reserve,
              int index);

    /** Destructor */
    ~MSHRQueue();

    /**
     * Find the first MSHR that matches the provided address.
     * @param addr The address to find.
     * @return Pointer to the matching MSHR, null if not found.
     */
    MSHR *findMatch(Addr addr) const;

    /**
     * Find and return all the matching entries in the provided vector.
     * @param addr The address to find.
     * @param matches The vector to return pointers to the matching entries.
     * @return True if any matches are found, false otherwise.
     * @todo Typedef the vector??
     */
    bool findMatches(Addr addr, std::vector<MSHR*>& matches) const;

    /**
     * Find any pending requests that overlap the given request.
     * @param pkt The request to find.
     * @return A pointer to the earliest matching MSHR.
     */
    MSHR *findPending(Addr addr, int size) const;

    bool checkFunctional(PacketPtr pkt, Addr blk_addr);

    /**
     * Allocates a new MSHR for the request and size. This places the request
     * as the first target in the MSHR.
     * @param pkt The request to handle.
     * @param size The number in bytes to fetch from memory.
     * @return The a pointer to the MSHR allocated.
     *
     * @pre There are free entries.
     */
    MSHR *allocate(Addr addr, int size, PacketPtr &pkt,
                   Tick when, Counter order);

    /**
     * Removes the given MSHR from the queue. This places the MSHR on the
     * free list.
     * @param mshr
     */
    void deallocate(MSHR *mshr);

    /**
     * Remove a MSHR from the queue. Returns an iterator into the
     * allocatedList for faster squash implementation.
     * @param mshr The MSHR to remove.
     * @return An iterator to the next entry in the allocatedList.
     */
    MSHR::Iterator deallocateOne(MSHR *mshr);

    /**
     * Moves the MSHR to the front of the pending list if it is not
     * in service.
     * @param mshr The entry to move.
     */
    void moveToFront(MSHR *mshr);

    /**
     * Mark the given MSHR as in service. This removes the MSHR from the
     * readyList. Deallocates the MSHR if it does not expect a response.
     * @param mshr The MSHR to mark in service.
     */
    void markInService(MSHR *mshr);

    /**
     * Mark an in service entry as pending, used to resend a request.
     * @param mshr The MSHR to resend.
     */
    void markPending(MSHR *mshr);

    /**
     * Squash outstanding requests with the given thread number. If a request
     * is in service, just squashes the targets.
     * @param threadNum The thread to squash.
     */
    void squash(int threadNum);

    /**
     * Returns true if the pending list is not empty.
     * @return True if there are outstanding requests.
     */
    bool havePending() const
    {
        return !readyList.empty();
    }

    /**
     * Returns true if there are no free entries.
     * @return True if this queue is full.
     */
    bool isFull() const
    {
        return (allocated > numEntries - numReserve);
    }

    /**
     * Returns the MSHR at the head of the readyList.
     * @return The next request to service.
     */
    MSHR *getNextMSHR() const
    {
        if (readyList.empty() || readyList.front()->readyTime > curTick) {
            return NULL;
        }
        return readyList.front();
    }

    Tick nextMSHRReadyTime() const
    {
        return readyList.empty() ? MaxTick : readyList.front()->readyTime;
    }
};

#endif //__MEM__CACHE__MISS__MSHR_QUEUE_HH__
