/*
 * Copyright (c) 2004, 2005, 206
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
 * This device acts as a simple time of date device. It's implemented as a
 * simple device register read.
 */

#ifndef __DEV_SPARC_DTOD_HH__
#define __DEV_SPARC_DTOD_HH__

#include <vector>

#include "base/range.hh"
#include "dev/io_device.hh"
#include "params/DumbTOD.hh"

/**
 * DumbTOD simply returns some idea of time when read.  Until we finish with
 * legion it starts with the start time and increments itself by 1000 each time.
 */
class DumbTOD : public BasicPioDevice
{
  private:
    uint64_t todTime;

  public:
    typedef DumbTODParams Params;
    DumbTOD(const Params *p);

    const Params *
    params() const
    {
        return dynamic_cast<const Params *>(_params);
    }

    virtual Tick read(PacketPtr pkt);
    virtual Tick write(PacketPtr pkt);

    /**
     * Serialize this object to the given output stream.
     * @param os The stream to serialize to.
     */
    virtual void serialize(std::ostream &os);

    /**
     * Reconstruct the state of this object from a checkpoint.
     * @param cp The checkpoint use.
     * @param section The section name of this object
     */
    virtual void unserialize(Checkpoint *cp, const std::string &section);


};

#endif // __DEV_BADDEV_HH__
