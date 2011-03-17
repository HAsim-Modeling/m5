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
 * Declaration of a simple bus bridge object with no buffering
 */

#ifndef __MEM_BRIDGE_HH__
#define __MEM_BRIDGE_HH__

#include <string>
#include <list>
#include <inttypes.h>
#include <queue>

#include "mem/mem_object.hh"
#include "mem/packet.hh"
#include "mem/port.hh"
#include "params/Bridge.hh"
#include "sim/eventq.hh"

class Bridge : public MemObject
{
  protected:
    /** Declaration of the buses port type, one will be instantiated for each
        of the interfaces connecting to the bus. */
    class BridgePort : public Port
    {
        /** A pointer to the bridge to which this port belongs. */
        Bridge *bridge;

        /**
         * Pointer to the port on the other side of the bridge
         * (connected to the other bus).
         */
        BridgePort *otherPort;

        /** Minimum delay though this bridge. */
        Tick delay;

        /** Min delay to respond to a nack. */
        Tick nackDelay;

        /** Pass ranges from one side of the bridge to the other? */
        std::vector<Range<Addr> > filterRanges;

        class PacketBuffer : public Packet::SenderState {

          public:
            Tick ready;
            PacketPtr pkt;
            bool nackedHere;
            Packet::SenderState *origSenderState;
            short origSrc;
            bool expectResponse;

            PacketBuffer(PacketPtr _pkt, Tick t, bool nack = false)
                : ready(t), pkt(_pkt), nackedHere(nack),
                  origSenderState(_pkt->senderState),
                  origSrc(nack ? _pkt->getDest() : _pkt->getSrc() ),
                  expectResponse(_pkt->needsResponse() && !nack)

            {
                if (!pkt->isResponse() && !nack)
                    pkt->senderState = this;
            }

            void fixResponse(PacketPtr pkt)
            {
                assert(pkt->senderState == this);
                pkt->setDest(origSrc);
                pkt->senderState = origSenderState;
            }
        };

        /**
         * Outbound packet queue.  Packets are held in this queue for a
         * specified delay to model the processing delay of the
         * bridge.
         */
        std::list<PacketBuffer*> sendQueue;

        int outstandingResponses;
        int queuedRequests;

        /** If we're waiting for a retry to happen.*/
        bool inRetry;

        /** Max queue size for outbound packets */
        int reqQueueLimit;

        /** Max queue size for reserved responses. */
        int respQueueLimit;

        /**
         * Is this side blocked from accepting outbound packets?
         */
        bool respQueueFull();
        bool reqQueueFull();

        void queueForSendTiming(PacketPtr pkt);

        void finishSend(PacketBuffer *buf);

        void nackRequest(PacketPtr pkt);

        /**
         * Handle send event, scheduled when the packet at the head of
         * the outbound queue is ready to transmit (for timing
         * accesses only).
         */
        void trySend();

        class SendEvent : public Event
        {
            BridgePort *port;

          public:
            SendEvent(BridgePort *p)
                : Event(&mainEventQueue), port(p) {}

            virtual void process() { port->trySend(); }

            virtual const char *description() const { return "bridge send"; }
        };

        SendEvent sendEvent;

      public:
        /** Constructor for the BusPort.*/
        BridgePort(const std::string &_name, Bridge *_bridge,
                BridgePort *_otherPort, int _delay, int _nack_delay,
                int _req_limit, int _resp_limit,
                std::vector<Range<Addr> > filter_ranges);

      protected:

        /** When receiving a timing request from the peer port,
            pass it to the bridge. */
        virtual bool recvTiming(PacketPtr pkt);

        /** When receiving a retry request from the peer port,
            pass it to the bridge. */
        virtual void recvRetry();

        /** When receiving a Atomic requestfrom the peer port,
            pass it to the bridge. */
        virtual Tick recvAtomic(PacketPtr pkt);

        /** When receiving a Functional request from the peer port,
            pass it to the bridge. */
        virtual void recvFunctional(PacketPtr pkt);

        /** When receiving a status changefrom the peer port,
            pass it to the bridge. */
        virtual void recvStatusChange(Status status);

        /** When receiving a address range request the peer port,
            pass it to the bridge. */
        virtual void getDeviceAddressRanges(AddrRangeList &resp,
                                            bool &snoop);
    };

    BridgePort portA, portB;

    /** If this bridge should acknowledge writes. */
    bool ackWrites;

  public:
    typedef BridgeParams Params;

  protected:
    Params *_params;

  public:
    const Params *params() const { return _params; }

    /** A function used to return the port associated with this bus object. */
    virtual Port *getPort(const std::string &if_name, int idx = -1);

    virtual void init();

    Bridge(Params *p);
};

#endif //__MEM_BUS_HH__
