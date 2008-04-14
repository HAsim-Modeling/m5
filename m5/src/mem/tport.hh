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

#ifndef __MEM_TPORT_HH__
#define __MEM_TPORT_HH__

/**
 * @file
 *
 * Declaration of SimpleTimingPort.
 */

#include "mem/port.hh"
#include "sim/eventq.hh"
#include <list>
#include <string>

/**
 * A simple port for interfacing objects that basically have only
 * functional memory behavior (e.g. I/O devices) to the memory system.
 * Both timing and functional accesses are implemented in terms of
 * atomic accesses.  A derived port class thus only needs to provide
 * recvAtomic() to support all memory access modes.
 *
 * The tricky part is handling recvTiming(), where the response must
 * be scheduled separately via a later call to sendTiming().  This
 * feature is handled by scheduling an internal event that calls
 * sendTiming() after a delay, and optionally rescheduling the
 * response if it is nacked.
 */
class SimpleTimingPort : public Port
{
  protected:
    /** A deferred packet, buffered to transmit later. */
    class DeferredPacket {
      public:
        Tick tick;      ///< The tick when the packet is ready to transmit
        PacketPtr pkt;  ///< Pointer to the packet to transmit
        DeferredPacket(Tick t, PacketPtr p)
            : tick(t), pkt(p)
        {}
    };

    typedef std::list<DeferredPacket> DeferredPacketList;
    typedef std::list<DeferredPacket>::iterator DeferredPacketIterator;

    /** A list of outgoing timing response packets that haven't been
     * serviced yet. */
    DeferredPacketList transmitList;

    /** This function attempts to send deferred packets.  Scheduled to
     * be called in the future via SendEvent. */
    void processSendEvent();

    /**
     * This class is used to implemented sendTiming() with a delay. When
     * a delay is requested a the event is scheduled if it isn't already.
     * When the event time expires it attempts to send the packet.
     * If it cannot, the packet sent when recvRetry() is called.
     **/
    typedef EventWrapper<SimpleTimingPort, &SimpleTimingPort::processSendEvent>
            SendEvent;

    Event *sendEvent;

    /** If we need to drain, keep the drain event around until we're done
     * here.*/
    Event *drainEvent;

    /** Remember whether we're awaiting a retry from the bus. */
    bool waitingOnRetry;

    /** Check the list of buffered packets against the supplied
     * functional request. */
    bool checkFunctional(PacketPtr funcPkt);

    /** Check whether we have a packet ready to go on the transmit list. */
    bool deferredPacketReady()
    { return !transmitList.empty() && transmitList.front().tick <= curTick; }

    Tick deferredPacketReadyTime()
    { return transmitList.empty() ? MaxTick : transmitList.front().tick; }

    void schedSendEvent(Tick when)
    {
        if (waitingOnRetry) {
            assert(!sendEvent->scheduled());
            return;
        }

        if (!sendEvent->scheduled()) {
            sendEvent->schedule(when);
        } else if (sendEvent->when() > when) {
            sendEvent->reschedule(when);
        }
    }


    /** Schedule a sendTiming() event to be called in the future.
     * @param pkt packet to send
     * @param absolute time (in ticks) to send packet
     */
    void schedSendTiming(PacketPtr pkt, Tick when);

    /** Attempt to send the packet at the head of the deferred packet
     * list.  Caller must guarantee that the deferred packet list is
     * non-empty and that the head packet is scheduled for curTick (or
     * earlier).
     */
    void sendDeferredPacket();

    /** This function is notification that the device should attempt to send a
     * packet again. */
    virtual void recvRetry();

    /** Implemented using recvAtomic(). */
    void recvFunctional(PacketPtr pkt);

    /** Implemented using recvAtomic(). */
    bool recvTiming(PacketPtr pkt);

    /**
     * Simple ports generally don't care about any status
     * changes... can always override this in cases where that's not
     * true. */
    virtual void recvStatusChange(Status status) { }


  public:

    SimpleTimingPort(std::string pname, MemObject *_owner = NULL)
        : Port(pname, _owner),
          sendEvent(new SendEvent(this)),
          drainEvent(NULL),
          waitingOnRetry(false)
    {}

    ~SimpleTimingPort() { delete sendEvent; }

    /** Hook for draining timing accesses from the system.  The
     * associated SimObject's drain() functions should be implemented
     * something like this when this class is used:
     \code
          PioDevice::drain(Event *de)
          {
              unsigned int count;
              count = SimpleTimingPort->drain(de);
              if (count)
                  changeState(Draining);
              else
                  changeState(Drained);
              return count;
          }
     \endcode
    */
    unsigned int drain(Event *de);
};

#endif // __MEM_TPORT_HH__
