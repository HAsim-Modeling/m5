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
 * Definition of MSHRQueue class functions.
 */

#include "mem/cache/mshr_queue.hh"

using namespace std;

MSHRQueue::MSHRQueue(const std::string &_label,
                     int num_entries, int reserve, int _index)
    : label(_label),
      numEntries(num_entries + reserve - 1), numReserve(reserve),
      index(_index)
{
    allocated = 0;
    inServiceEntries = 0;
    registers = new MSHR[numEntries];
    for (int i = 0; i < numEntries; ++i) {
        registers[i].queue = this;
        freeList.push_back(&registers[i]);
    }
}

MSHRQueue::~MSHRQueue()
{
    delete [] registers;
}

MSHR *
MSHRQueue::findMatch(Addr addr) const
{
    MSHR::ConstIterator i = allocatedList.begin();
    MSHR::ConstIterator end = allocatedList.end();
    for (; i != end; ++i) {
        MSHR *mshr = *i;
        if (mshr->addr == addr) {
            return mshr;
        }
    }
    return NULL;
}

bool
MSHRQueue::findMatches(Addr addr, vector<MSHR*>& matches) const
{
    // Need an empty vector
    assert(matches.empty());
    bool retval = false;
    MSHR::ConstIterator i = allocatedList.begin();
    MSHR::ConstIterator end = allocatedList.end();
    for (; i != end; ++i) {
        MSHR *mshr = *i;
        if (mshr->addr == addr) {
            retval = true;
            matches.push_back(mshr);
        }
    }
    return retval;
}


bool
MSHRQueue::checkFunctional(PacketPtr pkt, Addr blk_addr)
{
    pkt->pushLabel(label);
    MSHR::ConstIterator i = allocatedList.begin();
    MSHR::ConstIterator end = allocatedList.end();
    for (; i != end; ++i) {
        MSHR *mshr = *i;
        if (mshr->addr == blk_addr && mshr->checkFunctional(pkt)) {
            pkt->popLabel();
            return true;
        }
    }
    pkt->popLabel();
    return false;
}


MSHR *
MSHRQueue::findPending(Addr addr, int size) const
{
    MSHR::ConstIterator i = readyList.begin();
    MSHR::ConstIterator end = readyList.end();
    for (; i != end; ++i) {
        MSHR *mshr = *i;
        if (mshr->addr < addr) {
            if (mshr->addr + mshr->size > addr) {
                return mshr;
            }
        } else {
            if (addr + size > mshr->addr) {
                return mshr;
            }
        }
    }
    return NULL;
}


MSHR::Iterator
MSHRQueue::addToReadyList(MSHR *mshr)
{
    if (readyList.empty() || readyList.back()->readyTime <= mshr->readyTime) {
        return readyList.insert(readyList.end(), mshr);
    }

    MSHR::Iterator i = readyList.begin();
    MSHR::Iterator end = readyList.end();
    for (; i != end; ++i) {
        if ((*i)->readyTime > mshr->readyTime) {
            return readyList.insert(i, mshr);
        }
    }
    assert(false);
    return end;  // keep stupid compilers happy
}


MSHR *
MSHRQueue::allocate(Addr addr, int size, PacketPtr &pkt,
                    Tick when, Counter order)
{
    assert(!freeList.empty());
    MSHR *mshr = freeList.front();
    assert(mshr->getNumTargets() == 0);
    freeList.pop_front();

    mshr->allocate(addr, size, pkt, when, order);
    mshr->allocIter = allocatedList.insert(allocatedList.end(), mshr);
    mshr->readyIter = addToReadyList(mshr);

    allocated += 1;
    return mshr;
}


void
MSHRQueue::deallocate(MSHR *mshr)
{
    deallocateOne(mshr);
}

MSHR::Iterator
MSHRQueue::deallocateOne(MSHR *mshr)
{
    MSHR::Iterator retval = allocatedList.erase(mshr->allocIter);
    freeList.push_front(mshr);
    allocated--;
    if (mshr->inService) {
        inServiceEntries--;
    } else {
        readyList.erase(mshr->readyIter);
    }
    mshr->deallocate();
    return retval;
}

void
MSHRQueue::moveToFront(MSHR *mshr)
{
    if (!mshr->inService) {
        assert(mshr == *(mshr->readyIter));
        readyList.erase(mshr->readyIter);
        mshr->readyIter = readyList.insert(readyList.begin(), mshr);
    }
}

void
MSHRQueue::markInService(MSHR *mshr)
{
    if (mshr->markInService()) {
        deallocate(mshr);
    } else {
        readyList.erase(mshr->readyIter);
        inServiceEntries += 1;
    }
}

void
MSHRQueue::markPending(MSHR *mshr)
{
    assert(mshr->inService);
    mshr->inService = false;
    --inServiceEntries;
    /**
     * @ todo might want to add rerequests to front of pending list for
     * performance.
     */
    mshr->readyIter = addToReadyList(mshr);
}

void
MSHRQueue::squash(int threadNum)
{
    MSHR::Iterator i = allocatedList.begin();
    MSHR::Iterator end = allocatedList.end();
    for (; i != end;) {
        MSHR *mshr = *i;
        if (mshr->threadNum == threadNum) {
            while (mshr->hasTargets()) {
                mshr->popTarget();
                assert(0/*target->req->getThreadNum()*/ == threadNum);
            }
            assert(!mshr->hasTargets());
            assert(mshr->ntargets==0);
            if (!mshr->inService) {
                i = deallocateOne(mshr);
            } else {
                //mshr->pkt->flags &= ~CACHE_LINE_FILL;
                ++i;
            }
        } else {
            ++i;
        }
    }
}
