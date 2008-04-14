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
 * Authors: Steven K. Reinhardt
 */

#ifndef __CPU_SIMPLE_ATOMIC_HH__
#define __CPU_SIMPLE_ATOMIC_HH__

#include "cpu/simple/base.hh"

class AtomicSimpleCPU : public BaseSimpleCPU
{
  public:

    struct Params : public BaseSimpleCPU::Params {
        int width;
        bool simulate_stalls;
    };

    AtomicSimpleCPU(Params *params);
    virtual ~AtomicSimpleCPU();

    virtual void init();

  public:
    //
    enum Status {
        Running,
        Idle,
        SwitchedOut
    };

  protected:
    Status _status;

    Status status() const { return _status; }

  private:

    struct TickEvent : public Event
    {
        AtomicSimpleCPU *cpu;

        TickEvent(AtomicSimpleCPU *c);
        void process();
        const char *description() const;
    };

    TickEvent tickEvent;

    const int width;
    const bool simulate_stalls;

    // main simulation loop (one cycle)
    void tick();

    class CpuPort : public Port
    {
      public:

        CpuPort(const std::string &_name, AtomicSimpleCPU *_cpu)
            : Port(_name, _cpu), cpu(_cpu)
        { }

        bool snoopRangeSent;

      protected:

        AtomicSimpleCPU *cpu;

        virtual bool recvTiming(PacketPtr pkt);

        virtual Tick recvAtomic(PacketPtr pkt);

        virtual void recvFunctional(PacketPtr pkt);

        virtual void recvStatusChange(Status status);

        virtual void recvRetry();

        virtual void getDeviceAddressRanges(AddrRangeList &resp,
            bool &snoop)
        { resp.clear(); snoop = true; }

    };
    CpuPort icachePort;

    class DcachePort : public CpuPort
    {
      public:
        DcachePort(const std::string &_name, AtomicSimpleCPU *_cpu)
            : CpuPort(_name, _cpu)
        { }

        virtual void setPeer(Port *port);
    };
    DcachePort dcachePort;

    CpuPort physmemPort;
    bool hasPhysMemPort;
    Request ifetch_req;
    Request data_read_req;
    Request data_write_req;

    bool dcache_access;
    Tick dcache_latency;
    
    Range<Addr> physMemAddr;

  public:

    virtual Port *getPort(const std::string &if_name, int idx = -1);

    virtual void serialize(std::ostream &os);
    virtual void unserialize(Checkpoint *cp, const std::string &section);
    virtual void resume();

    void switchOut();
    void takeOverFrom(BaseCPU *oldCPU);

    virtual void activateContext(int thread_num, int delay);
    virtual void suspendContext(int thread_num);

    template <class T>
    Fault read(Addr addr, T &data, unsigned flags);

    template <class T>
    Fault write(T data, Addr addr, unsigned flags, uint64_t *res);

    Fault translateDataReadAddr(Addr vaddr, Addr &paddr,
            int size, unsigned flags);
    Fault translateDataWriteAddr(Addr vaddr, Addr &paddr,
            int size, unsigned flags);

    /**
     * Print state of address in memory system via PrintReq (for
     * debugging).
     */
    void printAddr(Addr a);
};

#endif // __CPU_SIMPLE_ATOMIC_HH__
