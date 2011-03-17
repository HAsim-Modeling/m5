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
 * Declaration of a fake device.
 */

#ifndef __ISA_FAKE_HH__
#define __ISA_FAKE_HH__

#include <string>

#include "base/range.hh"
#include "dev/io_device.hh"
// #include "dev/alpha/tsunami.hh"
#include "params/IsaFake.hh"
#include "mem/packet.hh"

/**
 * IsaFake is a device that returns, BadAddr, 1 or 0 on all reads and
 *  rites. It is meant to be placed at an address range
 * so that an mcheck doesn't occur when an os probes a piece of hw
 * that doesn't exist (e.g. UARTs1-3), or catch requests in the memory system
 * that have no responders..
 */
class IsaFake : public BasicPioDevice
{
  protected:
    uint8_t retData8;
    uint16_t retData16;
    uint32_t retData32;
    uint64_t retData64;

  public:
    typedef IsaFakeParams Params;
    const Params *
    params() const
    {
        return dynamic_cast<const Params *>(_params);
    }
    /**
      * The constructor for Isa Fake just registers itself with the MMU.
      * @param p params structure
      */
    IsaFake(Params *p);

    /**
     * This read always returns -1.
     * @param pkt The memory request.
     * @param data Where to put the data.
     */
    virtual Tick read(PacketPtr pkt);

    /**
     * All writes are simply ignored.
     * @param pkt The memory request.
     * @param data the data to not write.
     */
    virtual Tick write(PacketPtr pkt);
};

#endif // __ISA_FAKE_HH__
