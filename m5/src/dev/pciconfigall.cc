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
 * Authors: Andrew L. Schultz
 *          Ali G. Saidi
 */

/* @file
 * PCI Configspace implementation
 */

#include "base/trace.hh"
#include "dev/pciconfigall.hh"
#include "dev/pcireg.h"
#include "dev/platform.hh"
#include "mem/packet.hh"
#include "mem/packet_access.hh"
#include "params/PciConfigAll.hh"
#include "sim/system.hh"

using namespace std;

PciConfigAll::PciConfigAll(const Params *p)
    : PioDevice(p)
{
    pioAddr = p->platform->calcConfigAddr(params()->bus,0,0);
}


Tick
PciConfigAll::read(PacketPtr pkt)
{

    pkt->allocate();

    DPRINTF(PciConfigAll, "read  va=%#x size=%d\n", pkt->getAddr(),
            pkt->getSize());

    switch (pkt->getSize()) {
      case sizeof(uint32_t):
         pkt->set<uint32_t>(0xFFFFFFFF);
         break;
      case sizeof(uint16_t):
         pkt->set<uint16_t>(0xFFFF);
         break;
      case sizeof(uint8_t):
         pkt->set<uint8_t>(0xFF);
         break;
      default:
        panic("invalid access size(?) for PCI configspace!\n");
    }
    pkt->makeAtomicResponse();
    return params()->pio_latency;
}

Tick
PciConfigAll::write(PacketPtr pkt)
{
    panic("Attempting to write to config space on non-existant device\n");
    M5_DUMMY_RETURN
}


void
PciConfigAll::addressRanges(AddrRangeList &range_list)
{
    range_list.clear();
    range_list.push_back(RangeSize(pioAddr, params()->size));
}


#ifndef DOXYGEN_SHOULD_SKIP_THIS

PciConfigAll *
PciConfigAllParams::create()
{
    return new PciConfigAll(this);
}

#endif // DOXYGEN_SHOULD_SKIP_THIS
