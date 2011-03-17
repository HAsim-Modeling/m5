/*
 * Copyright (c) 2004, 2005
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

/** @file
 * Isa Fake Device implementation
 */

#include "base/trace.hh"
#include "dev/isa_fake.hh"
#include "mem/packet.hh"
#include "mem/packet_access.hh"
#include "sim/system.hh"

using namespace std;

IsaFake::IsaFake(Params *p)
    : BasicPioDevice(p)
{
    if (!p->ret_bad_addr)
        pioSize = p->pio_size;

    retData8 = p->ret_data8;
    retData16 = p->ret_data16;
    retData32 = p->ret_data32;
    retData64 = p->ret_data64;
}

Tick
IsaFake::read(PacketPtr pkt)
{

    pkt->makeAtomicResponse();
    if (params()->warn_access != "")
        warn("Device %s accessed by read to address %#x size=%d\n",
                name(), pkt->getAddr(), pkt->getSize());
    if (params()->ret_bad_addr) {
        DPRINTF(IsaFake, "read to bad address va=%#x size=%d\n",
                pkt->getAddr(), pkt->getSize());
        pkt->setBadAddress();
    } else {
        assert(pkt->getAddr() >= pioAddr && pkt->getAddr() < pioAddr + pioSize);
        DPRINTF(IsaFake, "read  va=%#x size=%d\n",
                pkt->getAddr(), pkt->getSize());
        switch (pkt->getSize()) {
          case sizeof(uint64_t):
             pkt->set(retData64);
             break;
          case sizeof(uint32_t):
             pkt->set(retData32);
             break;
          case sizeof(uint16_t):
             pkt->set(retData16);
             break;
          case sizeof(uint8_t):
             pkt->set(retData8);
             break;
          default:
            panic("invalid access size!\n");
        }
    }
    return pioDelay;
}

Tick
IsaFake::write(PacketPtr pkt)
{
    pkt->makeAtomicResponse();
    if (params()->warn_access != "") {
        uint64_t data;
        switch (pkt->getSize()) {
          case sizeof(uint64_t):
            data = pkt->get<uint64_t>();
            break;
          case sizeof(uint32_t):
            data = pkt->get<uint32_t>();
            break;
          case sizeof(uint16_t):
            data = pkt->get<uint16_t>();
            break;
          case sizeof(uint8_t):
            data = pkt->get<uint8_t>();
            break;
          default:
            panic("invalid access size!\n");
        }
        warn("Device %s accessed by write to address %#x size=%d data=%#x\n",
                name(), pkt->getAddr(), pkt->getSize(), data);
    }
    if (params()->ret_bad_addr) {
        DPRINTF(IsaFake, "write to bad address va=%#x size=%d \n",
                pkt->getAddr(), pkt->getSize());
        pkt->setBadAddress();
    } else {
        DPRINTF(IsaFake, "write - va=%#x size=%d \n",
                pkt->getAddr(), pkt->getSize());

        if (params()->update_data) {
            switch (pkt->getSize()) {
              case sizeof(uint64_t):
                retData64 = pkt->get<uint64_t>();
                break;
              case sizeof(uint32_t):
                retData32 = pkt->get<uint32_t>();
                break;
              case sizeof(uint16_t):
                retData16 = pkt->get<uint16_t>();
                break;
              case sizeof(uint8_t):
                retData8 = pkt->get<uint8_t>();
                break;
              default:
                panic("invalid access size!\n");
            }
        }
    }
    return pioDelay;
}

IsaFake *
IsaFakeParams::create()
{
    return new IsaFake(this);
}
