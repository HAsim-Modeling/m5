
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
 *          Steven K. Reinhardt
 */

/**
 * @file
 * Definition of a simple bus bridge without buffering.
 */

#include <algorithm>

#include "base/range_ops.hh"
#include "base/trace.hh"
#include "mem/bridge.hh"
#include "params/Bridge.hh"

Bridge::BridgePort::BridgePort(const std::string &_name,
                               Bridge *_bridge, BridgePort *_otherPort,
                               int _delay, int _nack_delay, int _req_limit,
                               int _resp_limit,
                               std::vector<Range<Addr> > filter_ranges)
    : Port(_name), bridge(_bridge), otherPort(_otherPort),
      delay(_delay), nackDelay(_nack_delay), filterRanges(filter_ranges),
      outstandingResponses(0), queuedRequests(0), inRetry(false),
      reqQueueLimit(_req_limit), respQueueLimit(_resp_limit), sendEvent(this)
{
}

Bridge::Bridge(Params *p)
    : MemObject(p),
      portA(p->name + "-portA", this, &portB, p->delay, p->nack_delay,
              p->req_size_a, p->resp_size_a, p->filter_ranges_a),
      portB(p->name + "-portB", this, &portA, p->delay, p->nack_delay,
              p->req_size_b, p->resp_size_b, p->filter_ranges_b),
      ackWrites(p->write_ack), _params(p)
{
    if (ackWrites)
        panic("No support for acknowledging writes\n");
}

Port *
Bridge::getPort(const std::string &if_name, int idx)
{
    BridgePort *port;

    if (if_name == "side_a")
        port = &portA;
    else if (if_name == "side_b")
        port = &portB;
    else
        return NULL;

    if (port->getPeer() != NULL && !port->getPeer()->isDefaultPort())
        panic("bridge side %s already connected to %s.",
                if_name, port->getPeer()->name());
    return port;
}


void
Bridge::init()
{
    // Make sure that both sides are connected to.
    if (portA.getPeer() == NULL || portB.getPeer() == NULL)
        fatal("Both ports of bus bridge are not connected to a bus.\n");

    if (portA.peerBlockSize() != portB.peerBlockSize())
        fatal("Busses don't have the same block size... Not supported.\n");
}

bool
Bridge::BridgePort::respQueueFull()
{
    assert(outstandingResponses >= 0 && outstandingResponses <= respQueueLimit);
    return outstandingResponses >= respQueueLimit;
}

bool
Bridge::BridgePort::reqQueueFull()
{
    assert(queuedRequests >= 0 && queuedRequests <= reqQueueLimit);
    return queuedRequests >= reqQueueLimit;
}

/** Function called by the port when the bus is receiving a Timing
 * transaction.*/
bool
Bridge::BridgePort::recvTiming(PacketPtr pkt)
{
    DPRINTF(BusBridge, "recvTiming: src %d dest %d addr 0x%x\n",
                pkt->getSrc(), pkt->getDest(), pkt->getAddr());

    DPRINTF(BusBridge, "Local queue size: %d outreq: %d outresp: %d\n",
                    sendQueue.size(), queuedRequests, outstandingResponses);
    DPRINTF(BusBridge, "Remote queue size: %d outreq: %d outresp: %d\n",
                    otherPort->sendQueue.size(), otherPort->queuedRequests,
                    otherPort->outstandingResponses);

    if (pkt->isRequest() && otherPort->reqQueueFull()) {
        DPRINTF(BusBridge, "Remote queue full, nacking\n");
        nackRequest(pkt);
        return true;
    }

    if (pkt->needsResponse())
        if (respQueueFull()) {
            DPRINTF(BusBridge, "Local queue full, no space for response, nacking\n");
            DPRINTF(BusBridge, "queue size: %d outreq: %d outstanding resp: %d\n",
                    sendQueue.size(), queuedRequests, outstandingResponses);
            nackRequest(pkt);
            return true;
        } else {
            DPRINTF(BusBridge, "Request Needs response, reserving space\n");
            ++outstandingResponses;
        }

    otherPort->queueForSendTiming(pkt);

    return true;
}

void
Bridge::BridgePort::nackRequest(PacketPtr pkt)
{
    // Nack the packet
    pkt->makeTimingResponse();
    pkt->setNacked();

    //put it on the list to send
    Tick readyTime = curTick + nackDelay;
    PacketBuffer *buf = new PacketBuffer(pkt, readyTime, true);

    // nothing on the list, add it and we're done
    if (sendQueue.empty()) {
        assert(!sendEvent.scheduled());
        sendEvent.schedule(readyTime);
        sendQueue.push_back(buf);
        return;
    }

    assert(sendEvent.scheduled() || inRetry);

    // does it go at the end?
    if (readyTime >= sendQueue.back()->ready) {
        sendQueue.push_back(buf);
        return;
    }

    // ok, somewhere in the middle, fun
    std::list<PacketBuffer*>::iterator i = sendQueue.begin();
    std::list<PacketBuffer*>::iterator end = sendQueue.end();
    std::list<PacketBuffer*>::iterator begin = sendQueue.begin();
    bool done = false;

    while (i != end && !done) {
        if (readyTime < (*i)->ready) {
            if (i == begin)
                sendEvent.reschedule(readyTime);
            sendQueue.insert(i,buf);
            done = true;
        }
        i++;
    }
    assert(done);
}


void
Bridge::BridgePort::queueForSendTiming(PacketPtr pkt)
{
    if (pkt->isResponse()) {
        // This is a response for a request we forwarded earlier.  The
        // corresponding PacketBuffer should be stored in the packet's
        // senderState field.

        PacketBuffer *buf = dynamic_cast<PacketBuffer*>(pkt->senderState);
        assert(buf != NULL);
        // set up new packet dest & senderState based on values saved
        // from original request
        buf->fixResponse(pkt);

        DPRINTF(BusBridge, "response, new dest %d\n", pkt->getDest());
        delete buf;
    }


    if (pkt->isRequest()) {
        ++queuedRequests;
    }



    Tick readyTime = curTick + delay;
    PacketBuffer *buf = new PacketBuffer(pkt, readyTime);

    // If we're about to put this packet at the head of the queue, we
    // need to schedule an event to do the transmit.  Otherwise there
    // should already be an event scheduled for sending the head
    // packet.
    if (sendQueue.empty()) {
        sendEvent.schedule(readyTime);
    }
    sendQueue.push_back(buf);
}

void
Bridge::BridgePort::trySend()
{
    assert(!sendQueue.empty());

    PacketBuffer *buf = sendQueue.front();

    assert(buf->ready <= curTick);

    PacketPtr pkt = buf->pkt;

    DPRINTF(BusBridge, "trySend: origSrc %d dest %d addr 0x%x\n",
            buf->origSrc, pkt->getDest(), pkt->getAddr());

    bool wasReq = pkt->isRequest();
    bool was_nacked_here = buf->nackedHere;

    // If the send was successful, make sure sender state was set to NULL
    // otherwise we could get a NACK back of a packet that didn't expect a
    // response and we would try to use freed memory.

    Packet::SenderState *old_sender_state = pkt->senderState;
    if (pkt->isRequest() && !buf->expectResponse)
        pkt->senderState = NULL;

    if (sendTiming(pkt)) {
        // send successful
        sendQueue.pop_front();
        buf->pkt = NULL; // we no longer own packet, so it's not safe to look at it

        if (buf->expectResponse) {
            // Must wait for response
            DPRINTF(BusBridge, "  successful: awaiting response (%d)\n",
                    outstandingResponses);
        } else {
            // no response expected... deallocate packet buffer now.
            DPRINTF(BusBridge, "  successful: no response expected\n");
            delete buf;
        }

        if (wasReq)
            --queuedRequests;
        else if (!was_nacked_here)
            --outstandingResponses;

        // If there are more packets to send, schedule event to try again.
        if (!sendQueue.empty()) {
            buf = sendQueue.front();
            DPRINTF(BusBridge, "Scheduling next send\n");
            sendEvent.schedule(std::max(buf->ready, curTick + 1));
        }
    } else {
        DPRINTF(BusBridge, "  unsuccessful\n");
        pkt->senderState = old_sender_state;
        inRetry = true;
    }

    DPRINTF(BusBridge, "trySend: queue size: %d outreq: %d outstanding resp: %d\n",
                    sendQueue.size(), queuedRequests, outstandingResponses);
}


void
Bridge::BridgePort::recvRetry()
{
    inRetry = false;
    Tick nextReady = sendQueue.front()->ready;
    if (nextReady <= curTick)
        trySend();
    else
        sendEvent.schedule(nextReady);
}

/** Function called by the port when the bus is receiving a Atomic
 * transaction.*/
Tick
Bridge::BridgePort::recvAtomic(PacketPtr pkt)
{
    return delay + otherPort->sendAtomic(pkt);
}

/** Function called by the port when the bus is receiving a Functional
 * transaction.*/
void
Bridge::BridgePort::recvFunctional(PacketPtr pkt)
{
    std::list<PacketBuffer*>::iterator i;

    pkt->pushLabel(name());

    for (i = sendQueue.begin();  i != sendQueue.end(); ++i) {
        if (pkt->checkFunctional((*i)->pkt))
            return;
    }

    pkt->popLabel();

    // fall through if pkt still not satisfied
    otherPort->sendFunctional(pkt);
}

/** Function called by the port when the bus is receiving a status change.*/
void
Bridge::BridgePort::recvStatusChange(Port::Status status)
{
    otherPort->sendStatusChange(status);
}

void
Bridge::BridgePort::getDeviceAddressRanges(AddrRangeList &resp,
                                           bool &snoop)
{
    otherPort->getPeerAddressRanges(resp, snoop);
    FilterRangeList(filterRanges, resp);
    // we don't allow snooping across bridges
    snoop = false;
}

Bridge *
BridgeParams::create()
{
    return new Bridge(this);
}
