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
 * Authors: Nathan L. Binkert
 */

#include "base/misc.hh"
#include "dev/pktfifo.hh"

using namespace std;

bool
PacketFifo::copyout(void *dest, int offset, int len)
{
    char *data = (char *)dest;
    if (offset + len >= size())
        return false;

    list<EthPacketPtr>::iterator p = fifo.begin();
    list<EthPacketPtr>::iterator end = fifo.end();
    while (len > 0) {
        while (offset >= (*p)->length) {
            offset -= (*p)->length;
            ++p;
        }

        if (p == end)
            panic("invalid fifo");

        int size = min((*p)->length - offset, len);
        memcpy(data, (*p)->data, size);
        offset = 0;
        len -= size;
        data += size;
        ++p;
    }

    return true;
}


void
PacketFifo::serialize(const string &base, ostream &os)
{
    paramOut(os, base + ".size", _size);
    paramOut(os, base + ".maxsize", _maxsize);
    paramOut(os, base + ".reserved", _reserved);
    paramOut(os, base + ".packets", fifo.size());

    int i = 0;
    list<EthPacketPtr>::iterator p = fifo.begin();
    list<EthPacketPtr>::iterator end = fifo.end();
    while (p != end) {
        (*p)->serialize(csprintf("%s.packet%d", base, i), os);
        ++p;
        ++i;
    }
}

void
PacketFifo::unserialize(const string &base, Checkpoint *cp,
                        const string &section)
{
    paramIn(cp, section, base + ".size", _size);
//  paramIn(cp, section, base + ".maxsize", _maxsize);
    paramIn(cp, section, base + ".reserved", _reserved);
    int fifosize;
    paramIn(cp, section, base + ".packets", fifosize);

    fifo.clear();

    for (int i = 0; i < fifosize; ++i) {
        EthPacketPtr p = new EthPacketData(16384);
        p->unserialize(csprintf("%s.packet%d", base, i), cp, section);
        fifo.push_back(p);
    }
}
