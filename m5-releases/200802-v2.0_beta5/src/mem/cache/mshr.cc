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
 *          David A. Greene
 */

/**
 * @file
 * Miss Status and Handling Register (MSHR) definitions.
 */

#include <assert.h>
#include <string>
#include <vector>
#include <algorithm>

#include "mem/cache/mshr.hh"
#include "sim/core.hh" // for curTick
#include "sim/host.hh"
#include "base/misc.hh"
#include "mem/cache/cache.hh"

using namespace std;

MSHR::MSHR()
{
    inService = false;
    ntargets = 0;
    threadNum = -1;
    targets = new TargetList();
    deferredTargets = new TargetList();
}


MSHR::TargetList::TargetList()
    : needsExclusive(false), hasUpgrade(false)
{}


inline void
MSHR::TargetList::add(PacketPtr pkt, Tick readyTime,
                      Counter order, bool cpuSide, bool markPending)
{
    if (cpuSide) {
        if (pkt->needsExclusive()) {
            needsExclusive = true;
        }

        if (pkt->cmd == MemCmd::UpgradeReq) {
            hasUpgrade = true;
        }
    }

    if (markPending) {
        MSHR *mshr = dynamic_cast<MSHR*>(pkt->senderState);
        if (mshr != NULL) {
            assert(!mshr->downstreamPending);
            mshr->downstreamPending = true;
        }
    }

    push_back(Target(pkt, readyTime, order, cpuSide, markPending));
}


void
MSHR::TargetList::replaceUpgrades()
{
    if (!hasUpgrade)
        return;

    Iterator end_i = end();
    for (Iterator i = begin(); i != end_i; ++i) {
        if (i->pkt->cmd == MemCmd::UpgradeReq) {
            i->pkt->cmd = MemCmd::ReadExReq;
            DPRINTF(Cache, "Replacing UpgradeReq with ReadExReq\n");
        }
    }

    hasUpgrade = false;
}


void
MSHR::TargetList::clearDownstreamPending()
{
    Iterator end_i = end();
    for (Iterator i = begin(); i != end_i; ++i) {
        if (i->markedPending) {
            MSHR *mshr = dynamic_cast<MSHR*>(i->pkt->senderState);
            if (mshr != NULL) {
                mshr->clearDownstreamPending();
            }
        }
    }
}


bool
MSHR::TargetList::checkFunctional(PacketPtr pkt)
{
    Iterator end_i = end();
    for (Iterator i = begin(); i != end_i; ++i) {
        if (pkt->checkFunctional(i->pkt)) {
            return true;
        }
    }

    return false;
}


void
MSHR::TargetList::
print(std::ostream &os, int verbosity, const std::string &prefix) const
{
    ConstIterator end_i = end();
    for (ConstIterator i = begin(); i != end_i; ++i) {
        ccprintf(os, "%s%s: ", prefix, i->isCpuSide() ? "cpu" : "mem");
        i->pkt->print(os, verbosity, "");
    }
}


void
MSHR::allocate(Addr _addr, int _size, PacketPtr target,
               Tick whenReady, Counter _order)
{
    addr = _addr;
    size = _size;
    readyTime = whenReady;
    order = _order;
    assert(target);
    isCacheFill = false;
    _isUncacheable = target->req->isUncacheable();
    inService = false;
    downstreamPending = false;
    threadNum = 0;
    ntargets = 1;
    // Don't know of a case where we would allocate a new MSHR for a
    // snoop (mem-side request), so set cpuSide to true here.
    assert(targets->isReset());
    targets->add(target, whenReady, _order, true, true);
    assert(deferredTargets->isReset());
    pendingInvalidate = false;
    pendingShared = false;
    data = NULL;
}


void
MSHR::clearDownstreamPending()
{
    assert(downstreamPending);
    downstreamPending = false;
    // recursively clear flag on any MSHRs we will be forwarding
    // responses to
    targets->clearDownstreamPending();
}

bool
MSHR::markInService()
{
    assert(!inService);
    if (isSimpleForward()) {
        // we just forwarded the request packet & don't expect a
        // response, so get rid of it
        assert(getNumTargets() == 1);
        popTarget();
        return true;
    }
    inService = true;
    if (!downstreamPending) {
        // let upstream caches know that the request has made it to a
        // level where it's going to get a response
        targets->clearDownstreamPending();
    }
    return false;
}


void
MSHR::deallocate()
{
    assert(targets->empty());
    targets->resetFlags();
    assert(deferredTargets->isReset());
    assert(ntargets == 0);
    inService = false;
    //allocIter = NULL;
    //readyIter = NULL;
}

/*
 * Adds a target to an MSHR
 */
void
MSHR::allocateTarget(PacketPtr pkt, Tick whenReady, Counter _order)
{
    // if there's a request already in service for this MSHR, we will
    // have to defer the new target until after the response if any of
    // the following are true:
    // - there are other targets already deferred
    // - there's a pending invalidate to be applied after the response
    //   comes back (but before this target is processed)
    // - the outstanding request is for a non-exclusive block and this
    //   target requires an exclusive block
    if (inService &&
        (!deferredTargets->empty() || pendingInvalidate ||
         (!targets->needsExclusive && pkt->needsExclusive()))) {
        // need to put on deferred list
        deferredTargets->add(pkt, whenReady, _order, true, true);
    } else {
        // No request outstanding, or still OK to append to
        // outstanding request: append to regular target list.  Only
        // mark pending if current request hasn't been issued yet
        // (isn't in service).
        targets->add(pkt, whenReady, _order, true, !inService);
    }

    ++ntargets;
}

bool
MSHR::handleSnoop(PacketPtr pkt, Counter _order)
{
    if (!inService || (pkt->isExpressSnoop() && downstreamPending)) {
        // Request has not been issued yet, or it's been issued
        // locally but is buffered unissued at some downstream cache
        // which is forwarding us this snoop.  Either way, the packet
        // we're snooping logically precedes this MSHR's request, so
        // the snoop has no impact on the MSHR, but must be processed
        // in the standard way by the cache.  The only exception is
        // that if we're an L2+ cache buffering an UpgradeReq from a
        // higher-level cache, and the snoop is invalidating, then our
        // buffered upgrades must be converted to read exclusives,
        // since the upper-level cache no longer has a valid copy.
        // That is, even though the upper-level cache got out on its
        // local bus first, some other invalidating transaction
        // reached the global bus before the upgrade did.
        if (pkt->needsExclusive()) {
            targets->replaceUpgrades();
            deferredTargets->replaceUpgrades();
        }

        return false;
    }

    // From here on down, the request issued by this MSHR logically
    // precedes the request we're snooping.

    if (pkt->needsExclusive()) {
        // snooped request still precedes the re-request we'll have to
        // issue for deferred targets, if any...
        deferredTargets->replaceUpgrades();
    }

    if (pendingInvalidate) {
        // a prior snoop has already appended an invalidation, so
        // logically we don't have the block anymore; no need for
        // further snooping.
        return true;
    }

    if (targets->needsExclusive || pkt->needsExclusive()) {
        // actual target device (typ. PhysicalMemory) will delete the
        // packet on reception, so we need to save a copy here
        PacketPtr cp_pkt = new Packet(pkt, true);
        targets->add(cp_pkt, curTick, _order, false,
                     downstreamPending && targets->needsExclusive);
        ++ntargets;

        if (targets->needsExclusive) {
            // We're awaiting an exclusive copy, so ownership is pending.
            // It's up to us to respond once the data arrives.
            pkt->assertMemInhibit();
            pkt->setSupplyExclusive();
        } else {
            // Someone else may respond before we get around to
            // processing this snoop, which means the copied request
            // pointer will no longer be valid
            cp_pkt->req = NULL;
        }

        if (pkt->needsExclusive()) {
            // This transaction will take away our pending copy
            pendingInvalidate = true;
        }
    } else {
        // Read to a read: no conflict, so no need to record as
        // target, but make sure neither reader thinks he's getting an
        // exclusive copy
        pendingShared = true;
        pkt->assertShared();
    }

    return true;
}


bool
MSHR::promoteDeferredTargets()
{
    assert(targets->empty());
    if (deferredTargets->empty()) {
        return false;
    }

    // swap targets & deferredTargets lists
    TargetList *tmp = targets;
    targets = deferredTargets;
    deferredTargets = tmp;

    assert(targets->size() == ntargets);

    // clear deferredTargets flags
    deferredTargets->resetFlags();

    pendingInvalidate = false;
    pendingShared = false;
    order = targets->front().order;
    readyTime = std::max(curTick, targets->front().readyTime);

    return true;
}


void
MSHR::handleFill(Packet *pkt, CacheBlk *blk)
{
    if (pendingShared) {
        // we snooped another read while this read was in
        // service... assert shared line on its behalf
        pkt->assertShared();
    }

    if (!pkt->sharedAsserted() && !pendingInvalidate
        && deferredTargets->needsExclusive) {
        // We got an exclusive response, but we have deferred targets
        // which are waiting to request an exclusive copy (not because
        // of a pending invalidate).  This can happen if the original
        // request was for a read-only (non-exclusive) block, but we
        // got an exclusive copy anyway because of the E part of the
        // MOESI/MESI protocol.  Since we got the exclusive copy
        // there's no need to defer the targets, so move them up to
        // the regular target list.
        assert(!targets->needsExclusive);
        targets->needsExclusive = true;
        // if any of the deferred targets were upper-level cache
        // requests marked downstreamPending, need to clear that
        assert(!downstreamPending);  // not pending here anymore
        deferredTargets->clearDownstreamPending();
        // this clears out deferredTargets too
        targets->splice(targets->end(), *deferredTargets);
        deferredTargets->resetFlags();
    }
}


bool
MSHR::checkFunctional(PacketPtr pkt)
{
    // For printing, we treat the MSHR as a whole as single entity.
    // For other requests, we iterate over the individual targets
    // since that's where the actual data lies.
    if (pkt->isPrint()) {
        pkt->checkFunctional(this, addr, size, NULL);
        return false;
    } else {
        return (targets->checkFunctional(pkt) ||
                deferredTargets->checkFunctional(pkt));
    }
}


void
MSHR::print(std::ostream &os, int verbosity, const std::string &prefix) const
{
    ccprintf(os, "%s[%x:%x] %s %s %s state: %s %s %s %s\n",
             prefix, addr, addr+size-1,
             isCacheFill ? "Fill" : "",
             needsExclusive() ? "Excl" : "",
             _isUncacheable ? "Unc" : "",
             inService ? "InSvc" : "",
             downstreamPending ? "DwnPend" : "",
             pendingInvalidate ? "PendInv" : "",
             pendingShared ? "PendShared" : "");

    ccprintf(os, "%s  Targets:\n", prefix);
    targets->print(os, verbosity, prefix + "    ");
    if (!deferredTargets->empty()) {
        ccprintf(os, "%s  Deferred Targets:\n", prefix);
        deferredTargets->print(os, verbosity, prefix + "      ");
    }
}

MSHR::~MSHR()
{
}