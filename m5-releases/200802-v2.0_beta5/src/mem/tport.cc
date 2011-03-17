/*
 * Copyright (c) 2006
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
 * Authors: Ali G. Saidi
 */

#include "mem/tport.hh"

bool
SimpleTimingPort::checkFunctional(PacketPtr pkt)
{
    DeferredPacketIterator i = transmitList.begin();
    DeferredPacketIterator end = transmitList.end();

    for (; i != end; ++i) {
        PacketPtr target = i->pkt;
        // If the target contains data, and it overlaps the
        // probed request, need to update data
        if (pkt->checkFunctional(target)) {
            return true;
        }
    }

    return false;
}

void
SimpleTimingPort::recvFunctional(PacketPtr pkt)
{
    if (!checkFunctional(pkt)) {
        // Just do an atomic access and throw away the returned latency
        recvAtomic(pkt);
    }
}

bool
SimpleTimingPort::recvTiming(PacketPtr pkt)
{
    // If the device is only a slave, it should only be sending
    // responses, which should never get nacked.  There used to be
    // code to hanldle nacks here, but I'm pretty sure it didn't work
    // correctly with the drain code, so that would need to be fixed
    // if we ever added it back.
    assert(pkt->isRequest());

    if (pkt->memInhibitAsserted()) {
        // snooper will supply based on copy of packet
        // still target's responsibility to delete packet
        delete pkt;
        return true;
    }

    bool needsResponse = pkt->needsResponse();
    Tick latency = recvAtomic(pkt);
    // turn packet around to go back to requester if response expected
    if (needsResponse) {
        // recvAtomic() should already have turned packet into
        // atomic response
        assert(pkt->isResponse());
        schedSendTiming(pkt, curTick + latency);
    } else {
        delete pkt->req;
        delete pkt;
    }

    return true;
}


void
SimpleTimingPort::schedSendTiming(PacketPtr pkt, Tick when)
{
    assert(when > curTick);
    assert(when < curTick + Clock::Int::ms);

    // Nothing is on the list: add it and schedule an event
    if (transmitList.empty() || when < transmitList.front().tick) {
        transmitList.push_front(DeferredPacket(when, pkt));
        schedSendEvent(when);
        return;
    }

    // list is non-empty and this is not the head, so event should
    // already be scheduled
    assert(waitingOnRetry ||
           (sendEvent->scheduled() && sendEvent->when() <= when));

    // list is non-empty & this belongs at the end
    if (when >= transmitList.back().tick) {
        transmitList.push_back(DeferredPacket(when, pkt));
        return;
    }

    // this belongs in the middle somewhere
    DeferredPacketIterator i = transmitList.begin();
    i++; // already checked for insertion at front
    DeferredPacketIterator end = transmitList.end();

    for (; i != end; ++i) {
        if (when < i->tick) {
            transmitList.insert(i, DeferredPacket(when, pkt));
            return;
        }
    }
    assert(false); // should never get here
}


void
SimpleTimingPort::sendDeferredPacket()
{
    assert(deferredPacketReady());
    // take packet off list here; if recvTiming() on the other side
    // calls sendTiming() back on us (like SimpleTimingCpu does), then
    // we get confused by having a non-active packet on transmitList
    DeferredPacket dp = transmitList.front();
    transmitList.pop_front();
    bool success = sendTiming(dp.pkt);

    if (success) {
        if (!transmitList.empty() && !sendEvent->scheduled()) {
            Tick time = transmitList.front().tick;
            sendEvent->schedule(time <= curTick ? curTick+1 : time);
        }

        if (transmitList.empty() && drainEvent) {
            drainEvent->process();
            drainEvent = NULL;
        }
    } else {
        // Unsuccessful, need to put back on transmitList.  Callee
        // should not have messed with it (since it didn't accept that
        // packet), so we can just push it back on the front.
        assert(!sendEvent->scheduled());
        transmitList.push_front(dp);
    }

    waitingOnRetry = !success;

    if (waitingOnRetry) {
        DPRINTF(Bus, "Send failed, waiting on retry\n");
    }
}


void
SimpleTimingPort::recvRetry()
{
    DPRINTF(Bus, "Received retry\n");
    assert(waitingOnRetry);
    sendDeferredPacket();
}


void
SimpleTimingPort::processSendEvent()
{
    assert(!waitingOnRetry);
    sendDeferredPacket();
}


unsigned int
SimpleTimingPort::drain(Event *de)
{
    if (transmitList.size() == 0)
        return 0;
    drainEvent = de;
    return 1;
}
