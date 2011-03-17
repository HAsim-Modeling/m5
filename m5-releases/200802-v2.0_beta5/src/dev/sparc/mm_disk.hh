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

#ifndef __DEV_SPARC_MM_DISK_HH__
#define __DEV_SPARC_MM_DISK_HH__

#include "base/range.hh"
#include "dev/io_device.hh"
#include "dev/disk_image.hh"
#include "params/MmDisk.hh"

class MmDisk : public BasicPioDevice
{
  private:
    DiskImage *image;
    off_t curSector;
    bool dirty;
    uint8_t diskData[SectorSize];

  public:
    typedef MmDiskParams Params;
    MmDisk(const Params *p);

    const Params *
    params() const
    {
        return dynamic_cast<const Params *>(_params);
    }

    virtual Tick read(PacketPtr pkt);
    virtual Tick write(PacketPtr pkt);

    virtual void serialize(std::ostream &os);
};

#endif //__DEV_SPARC_MM_DISK_HH__

