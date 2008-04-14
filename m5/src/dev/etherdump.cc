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
 */

/* @file
 * Simple object for creating a simple pcap style packet trace
 */

#include <sys/time.h>

#include <algorithm>
#include <string>

#include "base/misc.hh"
#include "base/output.hh"
#include "dev/etherdump.hh"
#include "sim/core.hh"

using std::string;

EtherDump::EtherDump(const Params *p)
    : SimObject(p), stream(simout.resolve(p->file).c_str()),
      maxlen(p->maxlen)
{
}

#define DLT_EN10MB		1       	// Ethernet (10Mb)
#define TCPDUMP_MAGIC		0xa1b2c3d4
#define PCAP_VERSION_MAJOR	2
#define PCAP_VERSION_MINOR	4

struct pcap_file_header {
    uint32_t magic;
    uint16_t version_major;
    uint16_t version_minor;
    int32_t thiszone;		// gmt to local correction
    uint32_t sigfigs;		// accuracy of timestamps
    uint32_t snaplen;		// max length saved portion of each pkt
    uint32_t linktype;		// data link type (DLT_*)
};

struct pcap_pkthdr {
    uint32_t seconds;
    uint32_t microseconds;
    uint32_t caplen;		// length of portion present
    uint32_t len;		// length this packet (off wire)
};

void
EtherDump::init()
{
    curtime = time(NULL);
    struct pcap_file_header hdr;
    hdr.magic = TCPDUMP_MAGIC;
    hdr.version_major = PCAP_VERSION_MAJOR;
    hdr.version_minor = PCAP_VERSION_MINOR;

    hdr.thiszone = -5 * 3600;
    hdr.snaplen = 1500;
    hdr.sigfigs = 0;
    hdr.linktype = DLT_EN10MB;

    stream.write(reinterpret_cast<char *>(&hdr), sizeof(hdr));

    /*
     * output an empty packet with the current time so that we know
     * when the simulation began.  This allows us to correlate packets
     * to sim_cycles.
     */
    pcap_pkthdr pkthdr;
    pkthdr.seconds = curtime;
    pkthdr.microseconds = 0;
    pkthdr.caplen = 0;
    pkthdr.len = 0;
    stream.write(reinterpret_cast<char *>(&pkthdr), sizeof(pkthdr));

    stream.flush();
}

void
EtherDump::dumpPacket(EthPacketPtr &packet)
{
    pcap_pkthdr pkthdr;
    pkthdr.seconds = curtime + (curTick / Clock::Int::s);
    pkthdr.microseconds = (curTick / Clock::Int::us) % ULL(1000000);
    pkthdr.caplen = std::min(packet->length, maxlen);
    pkthdr.len = packet->length;
    stream.write(reinterpret_cast<char *>(&pkthdr), sizeof(pkthdr));
    stream.write(reinterpret_cast<char *>(packet->data), pkthdr.caplen);
    stream.flush();
}

EtherDump *
EtherDumpParams::create()
{
    return new EtherDump(this);
}
