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
 *          Andrew L. Schultz
 *          Miguel J. Serrano
 */

/* @file
 * A single PCI device configuration space entry.
 */

#include <list>
#include <string>
#include <vector>

#include "base/inifile.hh"
#include "base/intmath.hh" // for isPowerOf2(
#include "base/misc.hh"
#include "base/str.hh"	// for to_number
#include "base/trace.hh"
#include "dev/pciconfigall.hh"
#include "dev/pcidev.hh"
#include "dev/alpha/tsunamireg.h"
#include "mem/packet.hh"
#include "mem/packet_access.hh"
#include "sim/byteswap.hh"
#include "sim/core.hh"

using namespace std;


PciDev::PciConfigPort::PciConfigPort(PciDev *dev, int busid, int devid,
        int funcid, Platform *p)
    : SimpleTimingPort(dev->name() + "-pciconf"), device(dev), platform(p),
      busId(busid), deviceId(devid), functionId(funcid)
{
    configAddr = platform->calcConfigAddr(busId, deviceId, functionId);
}


Tick
PciDev::PciConfigPort::recvAtomic(PacketPtr pkt)
{
    assert(pkt->getAddr() >= configAddr &&
           pkt->getAddr() < configAddr + PCI_CONFIG_SIZE);
    return pkt->isRead() ? device->readConfig(pkt) : device->writeConfig(pkt);
}

void
PciDev::PciConfigPort::getDeviceAddressRanges(AddrRangeList &resp,
                                              bool &snoop)
{
    snoop = false;;
    resp.push_back(RangeSize(configAddr, PCI_CONFIG_SIZE+1));
}


PciDev::PciDev(const Params *p)
    : DmaDevice(p), plat(p->platform), pioDelay(p->pio_latency),
      configDelay(p->config_latency), configPort(NULL)
{
    config.vendor = htole(p->VendorID);
    config.device = htole(p->DeviceID);
    config.command = htole(p->Command);
    config.status = htole(p->Status);
    config.revision = htole(p->Revision);
    config.progIF = htole(p->ProgIF);
    config.subClassCode = htole(p->SubClassCode);
    config.classCode = htole(p->ClassCode);
    config.cacheLineSize = htole(p->CacheLineSize);
    config.latencyTimer = htole(p->LatencyTimer);
    config.headerType = htole(p->HeaderType);
    config.bist = htole(p->BIST);

    config.baseAddr[0] = htole(p->BAR0);
    config.baseAddr[1] = htole(p->BAR1);
    config.baseAddr[2] = htole(p->BAR2);
    config.baseAddr[3] = htole(p->BAR3);
    config.baseAddr[4] = htole(p->BAR4);
    config.baseAddr[5] = htole(p->BAR5);
    config.cardbusCIS = htole(p->CardbusCIS);
    config.subsystemVendorID = htole(p->SubsystemVendorID);
    config.subsystemID = htole(p->SubsystemID);
    config.expansionROM = htole(p->ExpansionROM);
    config.reserved0 = 0;
    config.reserved1 = 0;
    config.interruptLine = htole(p->InterruptLine);
    config.interruptPin = htole(p->InterruptPin);
    config.minimumGrant = htole(p->MinimumGrant);
    config.maximumLatency = htole(p->MaximumLatency);

    BARSize[0] = p->BAR0Size;
    BARSize[1] = p->BAR1Size;
    BARSize[2] = p->BAR2Size;
    BARSize[3] = p->BAR3Size;
    BARSize[4] = p->BAR4Size;
    BARSize[5] = p->BAR5Size;

    for (int i = 0; i < 6; ++i) {
        uint32_t barsize = BARSize[i];
        if (barsize != 0 && !isPowerOf2(barsize)) {
            fatal("BAR %d size %d is not a power of 2\n", i, BARSize[i]);
        }
    }

    memset(BARAddrs, 0, sizeof(BARAddrs));

    plat->registerPciDevice(0, p->pci_dev, p->pci_func,
            letoh(config.interruptLine));
}

void
PciDev::init()
{
    if (!configPort)
        panic("pci config port not connected to anything!");
   configPort->sendStatusChange(Port::RangeChange);
   PioDevice::init();
}

unsigned int
PciDev::drain(Event *de)
{
    unsigned int count;
    count = pioPort->drain(de) + dmaPort->drain(de) + configPort->drain(de);
    if (count)
        changeState(Draining);
    else
        changeState(Drained);
    return count;
}

Tick
PciDev::readConfig(PacketPtr pkt)
{
    int offset = pkt->getAddr() & PCI_CONFIG_SIZE;
    if (offset >= PCI_DEVICE_SPECIFIC)
        panic("Device specific PCI config space not implemented!\n");

    pkt->allocate();

    switch (pkt->getSize()) {
      case sizeof(uint8_t):
        pkt->set<uint8_t>(config.data[offset]);
        DPRINTF(PCIDEV,
            "readConfig:  dev %#x func %#x reg %#x 1 bytes: data = %#x\n",
            params()->pci_dev, params()->pci_func, offset,
            (uint32_t)pkt->get<uint8_t>());
        break;
      case sizeof(uint16_t):
        pkt->set<uint16_t>(*(uint16_t*)&config.data[offset]);
        DPRINTF(PCIDEV,
            "readConfig:  dev %#x func %#x reg %#x 2 bytes: data = %#x\n",
            params()->pci_dev, params()->pci_func, offset,
            (uint32_t)pkt->get<uint16_t>());
        break;
      case sizeof(uint32_t):
        pkt->set<uint32_t>(*(uint32_t*)&config.data[offset]);
        DPRINTF(PCIDEV,
            "readConfig:  dev %#x func %#x reg %#x 4 bytes: data = %#x\n",
            params()->pci_dev, params()->pci_func, offset,
            (uint32_t)pkt->get<uint32_t>());
        break;
      default:
        panic("invalid access size(?) for PCI configspace!\n");
    }
    pkt->makeAtomicResponse();
    return configDelay;

}

void
PciDev::addressRanges(AddrRangeList &range_list)
{
    int x = 0;
    range_list.clear();
    for (x = 0; x < 6; x++)
        if (BARAddrs[x] != 0)
            range_list.push_back(RangeSize(BARAddrs[x],BARSize[x]));
}

Tick
PciDev::writeConfig(PacketPtr pkt)
{
    int offset = pkt->getAddr() & PCI_CONFIG_SIZE;
    if (offset >= PCI_DEVICE_SPECIFIC)
        panic("Device specific PCI config space not implemented!\n");

    switch (pkt->getSize()) {
      case sizeof(uint8_t):
        switch (offset) {
          case PCI0_INTERRUPT_LINE:
            config.interruptLine = pkt->get<uint8_t>();
          case PCI_CACHE_LINE_SIZE:
            config.cacheLineSize = pkt->get<uint8_t>();
          case PCI_LATENCY_TIMER:
            config.latencyTimer = pkt->get<uint8_t>();
            break;
          /* Do nothing for these read-only registers */
          case PCI0_INTERRUPT_PIN:
          case PCI0_MINIMUM_GRANT:
          case PCI0_MAXIMUM_LATENCY:
          case PCI_CLASS_CODE:
          case PCI_REVISION_ID:
            break;
          default:
            panic("writing to a read only register");
        }
        DPRINTF(PCIDEV,
            "writeConfig: dev %#x func %#x reg %#x 1 bytes: data = %#x\n",
            params()->pci_dev, params()->pci_func, offset,
            (uint32_t)pkt->get<uint8_t>());
        break;
      case sizeof(uint16_t):
        switch (offset) {
          case PCI_COMMAND:
            config.command = pkt->get<uint8_t>();
          case PCI_STATUS:
            config.status = pkt->get<uint8_t>();
          case PCI_CACHE_LINE_SIZE:
            config.cacheLineSize = pkt->get<uint8_t>();
            break;
          default:
            panic("writing to a read only register");
        }
        DPRINTF(PCIDEV,
            "writeConfig: dev %#x func %#x reg %#x 2 bytes: data = %#x\n",
            params()->pci_dev, params()->pci_func, offset,
            (uint32_t)pkt->get<uint16_t>());
        break;
      case sizeof(uint32_t):
        switch (offset) {
          case PCI0_BASE_ADDR0:
          case PCI0_BASE_ADDR1:
          case PCI0_BASE_ADDR2:
          case PCI0_BASE_ADDR3:
          case PCI0_BASE_ADDR4:
          case PCI0_BASE_ADDR5:
            {
                int barnum = BAR_NUMBER(offset);

                // convert BAR values to host endianness
                uint32_t he_old_bar = letoh(config.baseAddr[barnum]);
                uint32_t he_new_bar = letoh(pkt->get<uint32_t>());

                uint32_t bar_mask =
                    BAR_IO_SPACE(he_old_bar) ? BAR_IO_MASK : BAR_MEM_MASK;

                // Writing 0xffffffff to a BAR tells the card to set the
                // value of the bar to a bitmask indicating the size of
                // memory it needs
                if (he_new_bar == 0xffffffff) {
                    he_new_bar = ~(BARSize[barnum] - 1);
                } else {
                    // does it mean something special to write 0 to a BAR?
                    he_new_bar &= ~bar_mask;
                    if (he_new_bar) {
                        Addr space_base = BAR_IO_SPACE(he_old_bar) ?
                            TSUNAMI_PCI0_IO : TSUNAMI_PCI0_MEMORY;
                        BARAddrs[barnum] = he_new_bar + space_base;
                        pioPort->sendStatusChange(Port::RangeChange);
                    }
                }
                config.baseAddr[barnum] = htole((he_new_bar & ~bar_mask) |
                                                (he_old_bar & bar_mask));
            }
            break;

          case PCI0_ROM_BASE_ADDR:
            if (letoh(pkt->get<uint32_t>()) == 0xfffffffe)
                config.expansionROM = htole((uint32_t)0xffffffff);
            else
                config.expansionROM = pkt->get<uint32_t>();
            break;

          case PCI_COMMAND:
            // This could also clear some of the error bits in the Status
            // register. However they should never get set, so lets ignore
            // it for now
            config.command = pkt->get<uint32_t>();
            break;

          default:
            DPRINTF(PCIDEV, "Writing to a read only register");
        }
        DPRINTF(PCIDEV,
            "writeConfig: dev %#x func %#x reg %#x 4 bytes: data = %#x\n",
            params()->pci_dev, params()->pci_func, offset,
            (uint32_t)pkt->get<uint32_t>());
        break;
      default:
        panic("invalid access size(?) for PCI configspace!\n");
    }
    pkt->makeAtomicResponse();
    return configDelay;
}

void
PciDev::serialize(ostream &os)
{
    SERIALIZE_ARRAY(BARSize, sizeof(BARSize) / sizeof(BARSize[0]));
    SERIALIZE_ARRAY(BARAddrs, sizeof(BARAddrs) / sizeof(BARAddrs[0]));
    SERIALIZE_ARRAY(config.data, sizeof(config.data) / sizeof(config.data[0]));
}

void
PciDev::unserialize(Checkpoint *cp, const std::string &section)
{
    UNSERIALIZE_ARRAY(BARSize, sizeof(BARSize) / sizeof(BARSize[0]));
    UNSERIALIZE_ARRAY(BARAddrs, sizeof(BARAddrs) / sizeof(BARAddrs[0]));
    UNSERIALIZE_ARRAY(config.data,
                      sizeof(config.data) / sizeof(config.data[0]));
    pioPort->sendStatusChange(Port::RangeChange);

}

