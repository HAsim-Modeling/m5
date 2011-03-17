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

/** @file
 * This device acts as a disk similar to the memory mapped disk device
 * in legion. Any access is translated to an offset in the disk image.
 */

#include <cstring>

#include "base/trace.hh"
#include "dev/sparc/mm_disk.hh"
#include "dev/platform.hh"
#include "mem/port.hh"
#include "mem/packet_access.hh"
#include "sim/byteswap.hh"
#include "sim/system.hh"

MmDisk::MmDisk(const Params *p)
    : BasicPioDevice(p), image(p->image), curSector((off_t)-1), dirty(false)
{
    std::memset(&diskData, 0, SectorSize);
    pioSize = image->size() * SectorSize;
}

Tick
MmDisk::read(PacketPtr pkt)
{
    Addr accessAddr;
    off_t sector;
    off_t bytes_read;
    uint16_t d16;
    uint32_t d32;
    uint64_t d64;

    assert(pkt->getAddr() >= pioAddr && pkt->getAddr() < pioAddr + pioSize);
    accessAddr = pkt->getAddr() - pioAddr;

    sector = accessAddr / SectorSize;

    if (sector != curSector) {
        if (dirty) {
            bytes_read = image->write(diskData, curSector);
            assert(bytes_read == SectorSize);
        }
        bytes_read = image->read(diskData,  sector);
        assert(bytes_read == SectorSize);
        curSector = sector;
    }
    switch (pkt->getSize()) {
      case sizeof(uint8_t):
        pkt->set(diskData[accessAddr % SectorSize]);
        DPRINTF(IdeDisk, "reading byte %#x value= %#x\n", accessAddr, diskData[accessAddr %
                SectorSize]);
        break;
      case sizeof(uint16_t):
        memcpy(&d16, diskData + (accessAddr % SectorSize), 2);
        pkt->set(htobe(d32));
        DPRINTF(IdeDisk, "reading word %#x value= %#x\n", accessAddr, d16);
        break;
      case sizeof(uint32_t):
        memcpy(&d32, diskData + (accessAddr % SectorSize), 4);
        pkt->set(htobe(d32));
        DPRINTF(IdeDisk, "reading dword %#x value= %#x\n", accessAddr, d32);
        break;
      case sizeof(uint64_t):
        memcpy(&d64, diskData + (accessAddr % SectorSize), 8);
        pkt->set(htobe(d64));
        DPRINTF(IdeDisk, "reading qword %#x value= %#x\n", accessAddr, d64);
        break;
      default:
        panic("Invalid access size\n");
    }

    pkt->makeAtomicResponse();
    return pioDelay;
}

Tick
MmDisk::write(PacketPtr pkt)
{
    Addr accessAddr;
    off_t sector;
    off_t bytes_read;
    uint16_t d16;
    uint32_t d32;
    uint64_t d64;

    assert(pkt->getAddr() >= pioAddr && pkt->getAddr() < pioAddr + pioSize);
    accessAddr = pkt->getAddr() - pioAddr;

    sector = accessAddr / SectorSize;

    if (sector != curSector) {
        if (dirty) {
            bytes_read = image->write(diskData, curSector);
            assert(bytes_read == SectorSize);
        }
        bytes_read = image->read(diskData,  sector);
        assert(bytes_read == SectorSize);
        curSector = sector;
    }
    dirty = true;

    switch (pkt->getSize()) {
      case sizeof(uint8_t):
        diskData[accessAddr % SectorSize] = htobe(pkt->get<uint8_t>());
        DPRINTF(IdeDisk, "writing byte %#x value= %#x\n", accessAddr, diskData[accessAddr %
                SectorSize]);
        break;
      case sizeof(uint16_t):
        d16 = htobe(pkt->get<uint16_t>());
        memcpy(diskData + (accessAddr % SectorSize), &d16, 2);
        DPRINTF(IdeDisk, "writing word %#x value= %#x\n", accessAddr, d16);
        break;
      case sizeof(uint32_t):
        d32 = htobe(pkt->get<uint32_t>());
        memcpy(diskData + (accessAddr % SectorSize), &d32, 4);
        DPRINTF(IdeDisk, "writing dword %#x value= %#x\n", accessAddr, d32);
        break;
      case sizeof(uint64_t):
        d64 = htobe(pkt->get<uint64_t>());
        memcpy(diskData + (accessAddr % SectorSize), &d64, 8);
        DPRINTF(IdeDisk, "writing qword %#x value= %#x\n", accessAddr, d64);
        break;
      default:
        panic("Invalid access size\n");
    }

    pkt->makeAtomicResponse();
    return pioDelay;
}

void
MmDisk::serialize(std::ostream &os)
{
    // just write any dirty changes to the cow layer it will take care of
    // serialization
    int bytes_read;
    if (dirty) {
        bytes_read = image->write(diskData, curSector);
        assert(bytes_read == SectorSize);
    }
}

MmDisk *
MmDiskParams::create()
{
    return new MmDisk(this);
}
