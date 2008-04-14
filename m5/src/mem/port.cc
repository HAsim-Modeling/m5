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
 * Authors: Steven K. Reinhardt
 */

/**
 * @file
 * Port object definitions.
 */
#include <cstring>

#include "base/chunk_generator.hh"
#include "base/trace.hh"
#include "mem/mem_object.hh"
#include "mem/port.hh"

class defaultPeerPortClass: public Port
{
  protected:
    void blowUp()
    {
        fatal("Unconnected port!");
    }

  public:
    defaultPeerPortClass() : Port("default_port")
    {}

    bool recvTiming(PacketPtr)
    {
        blowUp();
        return false;
    }

    Tick recvAtomic(PacketPtr)
    {
        blowUp();
        return 0;
    }

    void recvFunctional(PacketPtr)
    {
        blowUp();
    }

    void recvStatusChange(Status)
    {
        blowUp();
    }

    int deviceBlockSize()
    {
        blowUp();
        return 0;
    }

    void getDeviceAddressRanges(AddrRangeList &, bool &)
    {
        blowUp();
    }

    bool isDefaultPort() { return true; }

} defaultPeerPort;

Port::Port() : peer(&defaultPeerPort), owner(NULL)
{
}

Port::Port(const std::string &_name, MemObject *_owner) :
    portName(_name), peer(&defaultPeerPort), owner(_owner)
{
}

void
Port::setPeer(Port *port)
{
    DPRINTF(Config, "setting peer to %s\n", port->name());
    peer = port;
}

void
Port::removeConn()
{
    if (peer->getOwner())
        peer->getOwner()->deletePortRefs(peer);
    peer = NULL;
}

void
Port::blobHelper(Addr addr, uint8_t *p, int size, MemCmd cmd)
{
    Request req;

    for (ChunkGenerator gen(addr, size, peerBlockSize());
         !gen.done(); gen.next()) {
        req.setPhys(gen.addr(), gen.size(), 0);
        Packet pkt(&req, cmd, Packet::Broadcast);
        pkt.dataStatic(p);
        sendFunctional(&pkt);
        p += gen.size();
    }
}

void
Port::writeBlob(Addr addr, uint8_t *p, int size)
{
    blobHelper(addr, p, size, MemCmd::WriteReq);
}

void
Port::readBlob(Addr addr, uint8_t *p, int size)
{
    blobHelper(addr, p, size, MemCmd::ReadReq);
}

void
Port::memsetBlob(Addr addr, uint8_t val, int size)
{
    // quick and dirty...
    uint8_t *buf = new uint8_t[size];

    std::memset(buf, val, size);
    blobHelper(addr, buf, size, MemCmd::WriteReq);

    delete [] buf;
}


void
Port::printAddr(Addr a)
{
    Request req(a, 1, 0);
    Packet pkt(&req, MemCmd::PrintReq, Packet::Broadcast);
    Packet::PrintReqState prs(std::cerr);
    pkt.senderState = &prs;

    sendFunctional(&pkt);
}
