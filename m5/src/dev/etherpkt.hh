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
 *          Lisa R. Hsu
 */

/* @file
 * Reference counted class containing ethernet packet data
 */

#ifndef __ETHERPKT_HH__
#define __ETHERPKT_HH__

#include <iosfwd>
#include <memory>
#include <assert.h>

#include "base/refcnt.hh"
#include "sim/host.hh"

/*
 * Reference counted class containing ethernet packet data
 */
class Checkpoint;
class EthPacketData : public RefCounted
{
  public:
    /*
     * Pointer to packet data will be deleted
     */
    uint8_t *data;

    /*
     * Length of the current packet
     */
    int length;

    /*
     * Extra space taken up by the packet in whatever data structure
     * it is in.
     *
     * NOTE: This can only be use by *one* data structure at a time!
     */
    int slack;

  public:
    EthPacketData() : data(NULL), length(0), slack(0)
    { }

    explicit EthPacketData(size_t size)
        : data(new uint8_t[size]), length(0), slack(0)
    { }

    EthPacketData(std::auto_ptr<uint8_t> d, int l, int s = 0)
        : data(d.release()), length(l), slack(s)
    { }

    ~EthPacketData() { if (data) delete [] data; }

  public:
    void serialize(const std::string &base, std::ostream &os);
    void unserialize(const std::string &base, Checkpoint *cp,
                     const std::string &section);
};

typedef RefCountingPtr<EthPacketData> EthPacketPtr;

#endif // __ETHERPKT_HH__
