/*
 * Copyright (c) 2004, 2005, 2006
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
 * Time of date device implementation
 */
#include <sys/time.h>

#include <deque>
#include <string>
#include <vector>

#include "base/time.hh"
#include "base/trace.hh"
#include "dev/sparc/dtod.hh"
#include "dev/platform.hh"
#include "mem/packet_access.hh"
#include "mem/port.hh"
#include "sim/system.hh"

using namespace std;
using namespace TheISA;

DumbTOD::DumbTOD(const Params *p)
    : BasicPioDevice(p)
{
    struct tm tm = p->time;
    char *tz;

    pioSize = 0x08;

    tz = getenv("TZ");
    setenv("TZ", "", 1);
    tzset();
    todTime = mktime(&tm);
    if (tz)
        setenv("TZ", tz, 1);
    else
        unsetenv("TZ");
    tzset();

    DPRINTFN("Real-time clock set to %s\n", asctime(&tm));
    DPRINTFN("Real-time clock set to %d\n", todTime);
}

Tick
DumbTOD::read(PacketPtr pkt)
{
    assert(pkt->getAddr() >= pioAddr && pkt->getAddr() < pioAddr + pioSize);
    assert(pkt->getSize() == 8);

    pkt->allocate();
    pkt->set(todTime);
    todTime += 1000;

    pkt->makeAtomicResponse();
    return pioDelay;
}

Tick
DumbTOD::write(PacketPtr pkt)
{
    panic("Dumb tod device doesn't support writes\n");
}

void
DumbTOD::serialize(std::ostream &os)
{
    SERIALIZE_SCALAR(todTime);
}

void
DumbTOD::unserialize(Checkpoint *cp, const std::string &section)
{
    UNSERIALIZE_SCALAR(todTime);
}

DumbTOD *
DumbTODParams::create()
{
    return new DumbTOD(this);
}
