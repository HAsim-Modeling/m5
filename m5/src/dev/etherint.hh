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
 * Class representing the actual interface between two ethernet
 * components.
 */

#ifndef __DEV_ETHERINT_HH__
#define __DEV_ETHERINT_HH__

#include <string>

#include "dev/etherpkt.hh"

/*
 * Class representing the actual interface between two ethernet
 * components.  These components are intended to attach to another
 * ethernet interface on one side and whatever device on the other.
 */
class EtherInt
{
  protected:
    mutable std::string portName;
    EtherInt *peer;

  public:
    EtherInt(const std::string &name)
        : portName(name), peer(NULL) {}
    virtual ~EtherInt() {}

    /** Return port name (for DPRINTF). */
    const std::string &name() const { return portName; }

    void setPeer(EtherInt *p);
    EtherInt* getPeer() { return peer; }

    void recvDone() { peer->sendDone(); }
    virtual void sendDone() = 0;

    bool sendPacket(EthPacketPtr packet)
    { return peer ? peer->recvPacket(packet) : true; }
    virtual bool recvPacket(EthPacketPtr packet) = 0;

    bool askBusy() {return peer->isBusy(); }
    virtual bool isBusy() { return false; }
};

#endif // __DEV_ETHERINT_HH__
