/*
 * Copyright (c) 2002, 2003, 2004, 2005
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

/**
 * @file
 * Miss Status and Handling Register (MSHR) declaration.
 */

#ifndef __MSHR_HH__
#define __MSHR_HH__

#include <list>

#include "base/printable.hh"
#include "mem/packet.hh"

class CacheBlk;
class MSHRQueue;

/**
 * Miss Status and handling Register. This class keeps all the information
 * needed to handle a cache miss including a list of target requests.
 */
class MSHR : public Packet::SenderState, public Printable
{

  public:

    class Target {
      public:
        Tick recvTime;  //!< Time when request was received (for stats)
        Tick readyTime; //!< Time when request is ready to be serviced
        Counter order;  //!< Global order (for memory consistency mgmt)
        PacketPtr pkt;  //!< Pending request packet.
        bool cpuSide;   //!< Did request come from cpu side or mem side?
        bool markedPending; //!< Did we mark upstream MSHR
                            //!<  as downstreamPending?

        bool isCpuSide() const { return cpuSide; }

        Target(PacketPtr _pkt, Tick _readyTime, Counter _order,
               bool _cpuSide, bool _markedPending)
            : recvTime(curTick), readyTime(_readyTime), order(_order),
              pkt(_pkt), cpuSide(_cpuSide), markedPending(_markedPending)
        {}
    };

    class TargetList : public std::list<Target> {
        /** Target list iterator. */
        typedef std::list<Target>::iterator Iterator;
        typedef std::list<Target>::const_iterator ConstIterator;

      public:
        bool needsExclusive;
        bool hasUpgrade;

        TargetList();
        void resetFlags() { needsExclusive = hasUpgrade = false; }
        bool isReset()    { return !needsExclusive && !hasUpgrade; }
        void add(PacketPtr pkt, Tick readyTime, Counter order,
                 bool cpuSide, bool markPending);
        void replaceUpgrades();
        void clearDownstreamPending();
        bool checkFunctional(PacketPtr pkt);
        void print(std::ostream &os, int verbosity,
                   const std::string &prefix) const;
    };

    /** A list of MSHRs. */
    typedef std::list<MSHR *> List;
    /** MSHR list iterator. */
    typedef List::iterator Iterator;
    /** MSHR list const_iterator. */
    typedef List::const_iterator ConstIterator;

    /** Pointer to queue containing this MSHR. */
    MSHRQueue *queue;

    /** Cycle when ready to issue */
    Tick readyTime;

    /** Order number assigned by the miss queue. */
    Counter order;

    /** Address of the request. */
    Addr addr;

    /** Size of the request. */
    int size;

    /** True if the request has been sent to the bus. */
    bool inService;

    /** True if we will be putting the returned block in the cache */
    bool isCacheFill;

    /** True if we need to get an exclusive copy of the block. */
    bool needsExclusive() const { return targets->needsExclusive; }

    /** True if the request is uncacheable */
    bool _isUncacheable;

    bool downstreamPending;

    bool pendingInvalidate;
    bool pendingShared;

    /** Thread number of the miss. */
    short threadNum;
    /** The number of currently allocated targets. */
    short ntargets;


    /** Data buffer (if needed).  Currently used only for pending
     * upgrade handling. */
    uint8_t *data;

    /**
     * Pointer to this MSHR on the ready list.
     * @sa MissQueue, MSHRQueue::readyList
     */
    Iterator readyIter;

    /**
     * Pointer to this MSHR on the allocated list.
     * @sa MissQueue, MSHRQueue::allocatedList
     */
    Iterator allocIter;

private:
    /** List of all requests that match the address */
    TargetList *targets;

    TargetList *deferredTargets;

public:

    bool isUncacheable() { return _isUncacheable; }

    /**
     * Allocate a miss to this MSHR.
     * @param cmd The requesting command.
     * @param addr The address of the miss.
     * @param asid The address space id of the miss.
     * @param size The number of bytes to request.
     * @param pkt  The original miss.
     */
    void allocate(Addr addr, int size, PacketPtr pkt,
                  Tick when, Counter _order);

    bool markInService();

    void clearDownstreamPending();

    /**
     * Mark this MSHR as free.
     */
    void deallocate();

    /**
     * Add a request to the list of targets.
     * @param target The target.
     */
    void allocateTarget(PacketPtr target, Tick when, Counter order);
    bool handleSnoop(PacketPtr target, Counter order);

    /** A simple constructor. */
    MSHR();
    /** A simple destructor. */
    ~MSHR();

    /**
     * Returns the current number of allocated targets.
     * @return The current number of allocated targets.
     */
    int getNumTargets() { return ntargets; }

    /**
     * Returns a pointer to the target list.
     * @return a pointer to the target list.
     */
    TargetList *getTargetList() { return targets; }

    /**
     * Returns true if there are targets left.
     * @return true if there are targets
     */
    bool hasTargets() { return !targets->empty(); }

    /**
     * Returns a reference to the first target.
     * @return A pointer to the first target.
     */
    Target *getTarget() { assert(hasTargets());  return &targets->front(); }

    /**
     * Pop first target.
     */
    void popTarget()
    {
        --ntargets;
        targets->pop_front();
    }

    bool isSimpleForward()
    {
        if (getNumTargets() != 1)
            return false;
        Target *tgt = getTarget();
        return tgt->isCpuSide() && !tgt->pkt->needsResponse();
    }

    bool promoteDeferredTargets();

    void handleFill(Packet *pkt, CacheBlk *blk);

    bool checkFunctional(PacketPtr pkt);

    /**
     * Prints the contents of this MSHR for debugging.
     */
    void print(std::ostream &os,
               int verbosity = 0,
               const std::string &prefix = "") const;
};

#endif //__MSHR_HH__
