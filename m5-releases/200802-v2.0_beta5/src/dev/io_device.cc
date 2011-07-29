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
 *          Nathan L. Binkert
 */

#include "base/chunk_generator.hh"
#include "base/trace.hh"
#include "dev/io_device.hh"
#include "sim/system.hh"


PioPort::PioPort(PioDevice *dev, System *s, std::string pname)
    : SimpleTimingPort(dev->name() + pname, dev), device(dev)
{ }


Tick
PioPort::recvAtomic(PacketPtr pkt)
{
    return pkt->isRead() ? device->read(pkt) : device->write(pkt);
}

void
PioPort::getDeviceAddressRanges(AddrRangeList &resp, bool &snoop)
{
    snoop = false;
    device->addressRanges(resp);
}


PioDevice::PioDevice(const Params *p)
    : MemObject(p), platform(p->platform), sys(p->system), pioPort(NULL)
{}

PioDevice::~PioDevice()
{
    if (pioPort)
        delete pioPort;
}

void
PioDevice::init()
{
    if (!pioPort)
        panic("Pio port not connected to anything!");
    pioPort->sendStatusChange(Port::RangeChange);
}


unsigned int
PioDevice::drain(Event *de)
{
    unsigned int count;
    count = pioPort->drain(de);
    if (count)
        changeState(Draining);
    else
        changeState(Drained);
    return count;
}

BasicPioDevice::BasicPioDevice(const Params *p)
    : PioDevice(p), pioAddr(p->pio_addr), pioSize(0),
      pioDelay(p->pio_latency)
{}

void
BasicPioDevice::addressRanges(AddrRangeList &range_list)
{
    assert(pioSize != 0);
    range_list.clear();
    range_list.push_back(RangeSize(pioAddr, pioSize));
}


DmaPort::DmaPort(DmaDevice *dev, System *s)
    : Port(dev->name() + "-dmaport", dev), device(dev), sys(s),
      pendingCount(0), actionInProgress(0), drainEvent(NULL),
      backoffTime(0), inRetry(false), backoffEvent(this)
{ }

bool
DmaPort::recvTiming(PacketPtr pkt)
{
    if (pkt->wasNacked()) {
        DPRINTF(DMA, "Received nacked %s addr %#x\n",
                pkt->cmdString(), pkt->getAddr());

        if (backoffTime < device->minBackoffDelay)
            backoffTime = device->minBackoffDelay;
        else if (backoffTime < device->maxBackoffDelay)
            backoffTime <<= 1;

        backoffEvent.reschedule(curTick + backoffTime, true);

        DPRINTF(DMA, "Backoff time set to %d ticks\n", backoffTime);

        pkt->reinitNacked();
        queueDma(pkt, true);
    } else if (pkt->senderState) {
        DmaReqState *state;
        backoffTime >>= 2;

        DPRINTF(DMA, "Received response %s addr %#x size %#x\n",
                pkt->cmdString(), pkt->getAddr(), pkt->req->getSize());
        state = dynamic_cast<DmaReqState*>(pkt->senderState);
        pendingCount--;

        assert(pendingCount >= 0);
        assert(state);

        state->numBytes += pkt->req->getSize();
        assert(state->totBytes >= state->numBytes);
        if (state->totBytes == state->numBytes) {
            state->completionEvent->process();
            delete state;
        }
        delete pkt->req;
        delete pkt;

        if (pendingCount == 0 && drainEvent) {
            drainEvent->process();
            drainEvent = NULL;
        }
    }  else {
        panic("Got packet without sender state... huh?\n");
    }

    return true;
}

DmaDevice::DmaDevice(const Params *p)
    : PioDevice(p), dmaPort(NULL), minBackoffDelay(p->min_backoff_delay),
      maxBackoffDelay(p->max_backoff_delay)
{ }


unsigned int
DmaDevice::drain(Event *de)
{
    unsigned int count;
    count = pioPort->drain(de) + dmaPort->drain(de);
    if (count)
        changeState(Draining);
    else
        changeState(Drained);
    return count;
}

unsigned int
DmaPort::drain(Event *de)
{
    if (pendingCount == 0)
        return 0;
    drainEvent = de;
    return 1;
}


void
DmaPort::recvRetry()
{
    assert(transmitList.size());
    PacketPtr pkt = transmitList.front();
    bool result = true;
    do {
        DPRINTF(DMA, "Retry on %s addr %#x\n",
                pkt->cmdString(), pkt->getAddr());
        result = sendTiming(pkt);
        if (result) {
            DPRINTF(DMA, "-- Done\n");
            transmitList.pop_front();
            inRetry = false;
        } else {
            inRetry = true;
            DPRINTF(DMA, "-- Failed, queued\n");
        }
    } while (!backoffTime &&  result && transmitList.size());

    if (transmitList.size() && backoffTime && !inRetry) {
        DPRINTF(DMA, "Scheduling backoff for %d\n", curTick+backoffTime);
        if (!backoffEvent.scheduled())
            backoffEvent.schedule(backoffTime+curTick);
    }
    DPRINTF(DMA, "TransmitList: %d, backoffTime: %d inRetry: %d es: %d\n",
            transmitList.size(), backoffTime, inRetry,
            backoffEvent.scheduled());
}


void
DmaPort::dmaAction(Packet::Command cmd, Addr addr, int size, Event *event,
                   uint8_t *data)
{
    assert(event);

    assert(device->getState() == SimObject::Running);

    DmaReqState *reqState = new DmaReqState(event, this, size);


    DPRINTF(DMA, "Starting DMA for addr: %#x size: %d sched: %d\n", addr, size,
            event->scheduled());
    for (ChunkGenerator gen(addr, size, peerBlockSize());
         !gen.done(); gen.next()) {
            Request *req = new Request(gen.addr(), gen.size(), 0);
            PacketPtr pkt = new Packet(req, cmd, Packet::Broadcast);

            // Increment the data pointer on a write
            if (data)
                pkt->dataStatic(data + gen.complete());

            pkt->senderState = reqState;

            assert(pendingCount >= 0);
            pendingCount++;
            DPRINTF(DMA, "--Queuing DMA for addr: %#x size: %d\n", gen.addr(),
                    gen.size());
            queueDma(pkt);
    }

}

void
DmaPort::queueDma(PacketPtr pkt, bool front)
{

    if (front)
        transmitList.push_front(pkt);
    else
        transmitList.push_back(pkt);
    sendDma();
}


void
DmaPort::sendDma()
{
    // some kind of selction between access methods
    // more work is going to have to be done to make
    // switching actually work
    assert(transmitList.size());
    PacketPtr pkt = transmitList.front();

    Enums::MemoryMode state = sys->getMemoryMode();
    if (state == Enums::timing) {
        if (backoffEvent.scheduled() || inRetry) {
            DPRINTF(DMA, "Can't send immediately, waiting for retry or backoff timer\n");
            return;
        }

        DPRINTF(DMA, "Attempting to send %s addr %#x\n",
                pkt->cmdString(), pkt->getAddr());

        bool result;
        do {
            result = sendTiming(pkt);
            if (result) {
                transmitList.pop_front();
                DPRINTF(DMA, "-- Done\n");
            } else {
                inRetry = true;
                DPRINTF(DMA, "-- Failed: queued\n");
            }
        } while (result && !backoffTime && transmitList.size());

        if (transmitList.size() && backoffTime && !inRetry &&
                !backoffEvent.scheduled()) {
            DPRINTF(DMA, "-- Scheduling backoff timer for %d\n",
                    backoffTime+curTick);
            backoffEvent.schedule(backoffTime+curTick);
        }
    } else if (state == Enums::atomic) {
        transmitList.pop_front();

        Tick lat;
        DPRINTF(DMA, "--Sending  DMA for addr: %#x size: %d\n",
                pkt->req->getPaddr(), pkt->req->getSize());
        lat = sendAtomic(pkt);
        assert(pkt->senderState);
        DmaReqState *state = dynamic_cast<DmaReqState*>(pkt->senderState);
        assert(state);
        state->numBytes += pkt->req->getSize();

        DPRINTF(DMA, "--Received response for  DMA for addr: %#x size: %d nb: %d, tot: %d sched %d\n",
                pkt->req->getPaddr(), pkt->req->getSize(), state->numBytes,
                state->totBytes, state->completionEvent->scheduled());

        if (state->totBytes == state->numBytes) {
            assert(!state->completionEvent->scheduled());
            state->completionEvent->schedule(curTick + lat);
            delete state;
            delete pkt->req;
        }
        pendingCount--;
        assert(pendingCount >= 0);
        delete pkt;

        if (pendingCount == 0 && drainEvent) {
            drainEvent->process();
            drainEvent = NULL;
        }

   } else
       panic("Unknown memory command state.");
}

DmaDevice::~DmaDevice()
{
    if (dmaPort)
        delete dmaPort;
}