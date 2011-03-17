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
 * Authors: Nathan L. Binkert
 */

/* @file
 * Interface to connect a simulated ethernet device to the real world
 */

#ifndef __ETHERTAP_HH__
#define __ETHERTAP_HH__

#include <queue>
#include <string>

#include "base/pollevent.hh"
#include "dev/etherobject.hh"
#include "dev/etherint.hh"
#include "dev/etherpkt.hh"
#include "params/EtherTap.hh"
#include "sim/eventq.hh"
#include "sim/sim_object.hh"

class TapEvent;
class TapListener;
class EtherTapInt;

/*
 * Interface to connect a simulated ethernet device to the real world
 */
class EtherTap : public EtherObject
{
  protected:
    friend class TapEvent;
    TapEvent *event;

  protected:
    friend class TapListener;
    TapListener *listener;
    int socket;
    char *buffer;
    int buflen;
    int32_t buffer_offset;
    int32_t data_len;

    EtherDump *dump;

    void attach(int fd);
    void detach();

  protected:
    std::string device;
    std::queue<EthPacketPtr> packetBuffer;
    EtherTapInt *interface;

    void process(int revent);
    void enqueue(EthPacketData *packet);
    void retransmit();

    /*
     */
    class TxEvent : public Event
    {
      protected:
        EtherTap *tap;

      public:
        TxEvent(EtherTap *_tap)
            : Event(&mainEventQueue), tap(_tap) {}
        void process() { tap->retransmit(); }
        virtual const char *description() const
            { return "EtherTap retransmit"; }
    };

    friend class TxEvent;
    TxEvent txEvent;

  public:
    typedef EtherTapParams Params;
    EtherTap(const Params *p);
    virtual ~EtherTap();

    const Params *
    params() const
    {
        return dynamic_cast<const Params *>(_params);
    }

    virtual EtherInt *getEthPort(const std::string &if_name, int idx);

    virtual bool recvPacket(EthPacketPtr packet);
    virtual void sendDone();

    virtual void serialize(std::ostream &os);
    virtual void unserialize(Checkpoint *cp, const std::string &section);
};

class EtherTapInt : public EtherInt
{
  private:
    EtherTap *tap;
  public:
    EtherTapInt(const std::string &name, EtherTap *t)
            : EtherInt(name), tap(t)
    { }

    virtual bool recvPacket(EthPacketPtr pkt) { return tap->recvPacket(pkt); }
    virtual void sendDone() { tap->sendDone(); }
};


#endif // __ETHERTAP_HH__
