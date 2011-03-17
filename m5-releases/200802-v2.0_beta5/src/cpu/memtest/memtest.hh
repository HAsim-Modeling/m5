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
 * Authors: Erik G. Hallnor
 *          Steven K. Reinhardt
 */

#ifndef __CPU_MEMTEST_MEMTEST_HH__
#define __CPU_MEMTEST_MEMTEST_HH__

#include <set>

#include "base/statistics.hh"
#include "params/MemTest.hh"
#include "sim/eventq.hh"
#include "sim/sim_exit.hh"
#include "sim/sim_object.hh"
#include "sim/stats.hh"
#include "mem/mem_object.hh"
#include "mem/port.hh"

class Packet;
class MemTest : public MemObject
{
  public:
    typedef MemTestParams Params;
    MemTest(const Params *p);

    virtual void init();

    // register statistics
    virtual void regStats();

    inline Tick ticks(int numCycles) const { return numCycles; }

    // main simulation loop (one cycle)
    void tick();

    virtual Port *getPort(const std::string &if_name, int idx = -1);

    /**
     * Print state of address in memory system via PrintReq (for
     * debugging).
     */
    void printAddr(Addr a);

  protected:
    class TickEvent : public Event
    {
      private:
        MemTest *cpu;
      public:
        TickEvent(MemTest *c)
            : Event(&mainEventQueue, CPU_Tick_Pri), cpu(c) {}
        void process() {cpu->tick();}
        virtual const char *description() const { return "MemTest tick"; }
    };

    TickEvent tickEvent;

    class CpuPort : public Port
    {
        MemTest *memtest;

      public:

        CpuPort(const std::string &_name, MemTest *_memtest)
            : Port(_name, _memtest), memtest(_memtest)
        { }

        bool snoopRangeSent;

      protected:

        virtual bool recvTiming(PacketPtr pkt);

        virtual Tick recvAtomic(PacketPtr pkt);

        virtual void recvFunctional(PacketPtr pkt);

        virtual void recvStatusChange(Status status);

        virtual void recvRetry();

        virtual void getDeviceAddressRanges(AddrRangeList &resp,
                                            bool &snoop)
        { resp.clear(); snoop = false; }
    };

    CpuPort cachePort;
    CpuPort funcPort;

    bool snoopRangeSent;

    class MemTestSenderState : public Packet::SenderState
    {
      public:
        /** Constructor. */
        MemTestSenderState(uint8_t *_data)
            : data(_data)
        { }

        // Hold onto data pointer
        uint8_t *data;
    };

    PacketPtr retryPkt;

    bool accessRetry;

    unsigned size;		// size of testing memory region

    unsigned percentReads;	// target percentage of read accesses
    unsigned percentFunctional;	// target percentage of functional accesses
    unsigned percentUncacheable;

    int id;

    std::set<unsigned> outstandingAddrs;

    unsigned blockSize;

    Addr blockAddrMask;

    Addr blockAddr(Addr addr)
    {
        return (addr & ~blockAddrMask);
    }

    Addr traceBlockAddr;

    Addr baseAddr1;		// fix this to option
    Addr baseAddr2;		// fix this to option
    Addr uncacheAddr;

    unsigned progressInterval;	// frequency of progress reports
    Tick nextProgressMessage;	// access # for next progress report

    unsigned percentSourceUnaligned;
    unsigned percentDestUnaligned;

    Tick noResponseCycles;

    uint64_t numReads;
    uint64_t maxLoads;

    bool atomic;

    Stats::Scalar<> numReadsStat;
    Stats::Scalar<> numWritesStat;
    Stats::Scalar<> numCopiesStat;

    // called by MemCompleteEvent::process()
    void completeRequest(PacketPtr pkt);

    void sendPkt(PacketPtr pkt);

    void doRetry();

    friend class MemCompleteEvent;
};

#endif // __CPU_MEMTEST_MEMTEST_HH__



