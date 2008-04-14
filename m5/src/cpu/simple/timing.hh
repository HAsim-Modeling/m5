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

#ifndef __CPU_SIMPLE_TIMING_HH__
#define __CPU_SIMPLE_TIMING_HH__

#include "cpu/simple/base.hh"

class TimingSimpleCPU : public BaseSimpleCPU
{
  public:

    struct Params : public BaseSimpleCPU::Params {
    };

    TimingSimpleCPU(Params *params);
    virtual ~TimingSimpleCPU();

    virtual void init();

  public:
    //
    enum Status {
        Idle,
        Running,
        IcacheRetry,
        IcacheWaitResponse,
        IcacheWaitSwitch,
        DcacheRetry,
        DcacheWaitResponse,
        DcacheWaitSwitch,
        SwitchedOut
    };

  protected:
    Status _status;

    Status status() const { return _status; }

    Event *drainEvent;

  private:

    class CpuPort : public Port
    {
      protected:
        TimingSimpleCPU *cpu;
        Tick lat;

      public:

        CpuPort(const std::string &_name, TimingSimpleCPU *_cpu, Tick _lat)
            : Port(_name, _cpu), cpu(_cpu), lat(_lat)
        { }

        bool snoopRangeSent;

      protected:

        virtual Tick recvAtomic(PacketPtr pkt);

        virtual void recvFunctional(PacketPtr pkt);

        virtual void recvStatusChange(Status status);

        virtual void getDeviceAddressRanges(AddrRangeList &resp,
                                            bool &snoop)
        { resp.clear(); snoop = false; }

        struct TickEvent : public Event
        {
            PacketPtr pkt;
            TimingSimpleCPU *cpu;

            TickEvent(TimingSimpleCPU *_cpu)
                :Event(&mainEventQueue), cpu(_cpu) {}
            const char *description() const { return "Timing CPU tick"; }
            void schedule(PacketPtr _pkt, Tick t);
        };

    };

    class IcachePort : public CpuPort
    {
      public:

        IcachePort(TimingSimpleCPU *_cpu, Tick _lat)
            : CpuPort(_cpu->name() + "-iport", _cpu, _lat), tickEvent(_cpu)
        { }

      protected:

        virtual bool recvTiming(PacketPtr pkt);

        virtual void recvRetry();

        struct ITickEvent : public TickEvent
        {

            ITickEvent(TimingSimpleCPU *_cpu)
                : TickEvent(_cpu) {}
            void process();
            const char *description() const { return "Timing CPU icache tick"; }
        };

        ITickEvent tickEvent;

    };

    class DcachePort : public CpuPort
    {
      public:

        DcachePort(TimingSimpleCPU *_cpu, Tick _lat)
            : CpuPort(_cpu->name() + "-dport", _cpu, _lat), tickEvent(_cpu)
        { }

        virtual void setPeer(Port *port);

      protected:

        virtual bool recvTiming(PacketPtr pkt);

        virtual void recvRetry();

        struct DTickEvent : public TickEvent
        {
            DTickEvent(TimingSimpleCPU *_cpu)
                : TickEvent(_cpu) {}
            void process();
            const char *description() const { return "Timing CPU dcache tick"; }
        };

        DTickEvent tickEvent;

    };

    IcachePort icachePort;
    DcachePort dcachePort;

    PacketPtr ifetch_pkt;
    PacketPtr dcache_pkt;

    Tick previousTick;

  public:

    virtual Port *getPort(const std::string &if_name, int idx = -1);

    virtual void serialize(std::ostream &os);
    virtual void unserialize(Checkpoint *cp, const std::string &section);

    virtual unsigned int drain(Event *drain_event);
    virtual void resume();

    void switchOut();
    void takeOverFrom(BaseCPU *oldCPU);

    virtual void activateContext(int thread_num, int delay);
    virtual void suspendContext(int thread_num);

    template <class T>
    Fault read(Addr addr, T &data, unsigned flags);

    Fault translateDataReadAddr(Addr vaddr, Addr &paddr,
            int size, unsigned flags);

    template <class T>
    Fault write(T data, Addr addr, unsigned flags, uint64_t *res);

    Fault translateDataWriteAddr(Addr vaddr, Addr &paddr,
            int size, unsigned flags);

    void fetch();
    void completeIfetch(PacketPtr );
    void completeDataAccess(PacketPtr );
    void advanceInst(Fault fault);

    /**
     * Print state of address in memory system via PrintReq (for
     * debugging).
     */
    void printAddr(Addr a);

  private:

    typedef EventWrapper<TimingSimpleCPU, &TimingSimpleCPU::fetch> FetchEvent;
    FetchEvent *fetchEvent;
        
    struct IprEvent : Event {
        Packet *pkt;
        TimingSimpleCPU *cpu;
        IprEvent(Packet *_pkt, TimingSimpleCPU *_cpu, Tick t);
        virtual void process();
        virtual const char *description() const;
    };

    void completeDrain();
};

#endif // __CPU_SIMPLE_TIMING_HH__
