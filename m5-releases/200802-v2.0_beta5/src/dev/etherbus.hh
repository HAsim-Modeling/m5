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
 * Authors: Nathan L. Binkert
 */

/* @file
 * Device module for modelling an ethernet hub
 */

#ifndef __ETHERBUS_H__
#define __ETHERBUS_H__

#include "sim/eventq.hh"
#include "dev/etherpkt.hh"
#include "dev/etherobject.hh"
#include "params/EtherBus.hh"
#include "sim/sim_object.hh"
#include "params/EtherBus.hh"

class EtherDump;
class EtherInt;
class EtherBus : public EtherObject
{
  protected:
    typedef std::list<EtherInt *> devlist_t;
    devlist_t devlist;
    double ticksPerByte;
    bool loopback;

  protected:
    class DoneEvent : public Event
    {
      protected:
        EtherBus *bus;

      public:
        DoneEvent(EventQueue *q, EtherBus *b)
            : Event(q), bus(b) {}
        virtual void process() { bus->txDone(); }
        virtual const char *description() const
            { return "ethernet bus completion"; }
    };

    DoneEvent event;
    EthPacketPtr packet;
    EtherInt *sender;
    EtherDump *dump;

  public:
    typedef EtherBusParams Params;
    EtherBus(const Params *p);
    virtual ~EtherBus() {}

    const Params *
    params() const
    {
        return dynamic_cast<const Params *>(_params);
    }

    void txDone();
    void reg(EtherInt *dev);
    bool busy() const { return (bool)packet; }
    bool send(EtherInt *sender, EthPacketPtr &packet);
    virtual EtherInt *getEthPort(const std::string &if_name, int idx);
};

#endif // __ETHERBUS_H__
