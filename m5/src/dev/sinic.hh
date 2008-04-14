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

#ifndef __DEV_SINIC_HH__
#define __DEV_SINIC_HH__

#include "base/inet.hh"
#include "base/statistics.hh"
#include "dev/etherint.hh"
#include "dev/etherpkt.hh"
#include "dev/io_device.hh"
#include "dev/pcidev.hh"
#include "dev/pktfifo.hh"
#include "dev/sinicreg.hh"
#include "params/Sinic.hh"
#include "sim/eventq.hh"

namespace Sinic {

class Interface;
class Base : public PciDev
{
  protected:
    bool rxEnable;
    bool txEnable;
    Tick clock;
    inline Tick ticks(int numCycles) const { return numCycles * clock; }

  protected:
    Tick intrDelay;
    Tick intrTick;
    bool cpuIntrEnable;
    bool cpuPendingIntr;
    void cpuIntrPost(Tick when);
    void cpuInterrupt();
    void cpuIntrClear();

    typedef EventWrapper<Base, &Base::cpuInterrupt> IntrEvent;
    friend void IntrEvent::process();
    IntrEvent *intrEvent;
    Interface *interface;

    bool cpuIntrPending() const;
    void cpuIntrAck() { cpuIntrClear(); }

/**
 * Serialization stuff
 */
  public:
    virtual void serialize(std::ostream &os);
    virtual void unserialize(Checkpoint *cp, const std::string &section);

/**
 * Construction/Destruction/Parameters
 */
  public:
    typedef SinicParams Params;
    const Params *params() const { return (const Params *)_params; }
    Base(const Params *p);
};

class Device : public Base
{
  protected:
    /** Receive State Machine States */
    enum RxState {
        rxIdle,
        rxFifoBlock,
        rxBeginCopy,
        rxCopy,
        rxCopyDone
    };

    /** Transmit State Machine states */
    enum TxState {
        txIdle,
        txFifoBlock,
        txBeginCopy,
        txCopy,
        txCopyDone
    };

    /** device register file */
    struct {
        uint32_t Config;       // 0x00
        uint32_t Command;      // 0x04
        uint32_t IntrStatus;   // 0x08
        uint32_t IntrMask;     // 0x0c
        uint32_t RxMaxCopy;    // 0x10
        uint32_t TxMaxCopy;    // 0x14
        uint32_t RxMaxIntr;    // 0x18
        uint32_t VirtualCount; // 0x1c
        uint32_t RxFifoSize;   // 0x20
        uint32_t TxFifoSize;   // 0x24
        uint32_t RxFifoMark;   // 0x28
        uint32_t TxFifoMark;   // 0x2c
        uint64_t RxData;       // 0x30
        uint64_t RxDone;       // 0x38
        uint64_t RxWait;       // 0x40
        uint64_t TxData;       // 0x48
        uint64_t TxDone;       // 0x50
        uint64_t TxWait;       // 0x58
        uint64_t HwAddr;       // 0x60
    } regs;

    struct VirtualReg {
        uint64_t RxData;
        uint64_t RxDone;
        uint64_t TxData;
        uint64_t TxDone;

        PacketFifo::iterator rxPacket;
        int rxPacketOffset;
        int rxPacketBytes;
        uint64_t rxDoneData;

        Counter rxUnique;
        Counter txUnique;

        VirtualReg()
            : RxData(0), RxDone(0), TxData(0), TxDone(0),
              rxPacketOffset(0), rxPacketBytes(0), rxDoneData(0)
        { }
    };
    typedef std::vector<VirtualReg> VirtualRegs;
    typedef std::list<int> VirtualList;
    Counter rxUnique;
    Counter txUnique;
    VirtualRegs virtualRegs;
    VirtualList rxList;
    VirtualList rxBusy;
    int rxActive;
    VirtualList txList;

    uint8_t  &regData8(Addr daddr) { return *((uint8_t *)&regs + daddr); }
    uint32_t &regData32(Addr daddr) { return *(uint32_t *)&regData8(daddr); }
    uint64_t &regData64(Addr daddr) { return *(uint64_t *)&regData8(daddr); }

  protected:
    RxState rxState;
    PacketFifo rxFifo;
    PacketFifo::iterator rxFifoPtr;
    bool rxEmpty;
    bool rxLow;
    Addr rxDmaAddr;
    uint8_t *rxDmaData;
    int rxDmaLen;

    TxState txState;
    PacketFifo txFifo;
    bool txFull;
    EthPacketPtr txPacket;
    int txPacketOffset;
    int txPacketBytes;
    Addr txDmaAddr;
    uint8_t *txDmaData;
    int txDmaLen;

  protected:
    void reset();

    void rxKick();
    Tick rxKickTick;
    typedef EventWrapper<Device, &Device::rxKick> RxKickEvent;
    friend void RxKickEvent::process();

    void txKick();
    Tick txKickTick;
    typedef EventWrapper<Device, &Device::txKick> TxKickEvent;
    friend void TxKickEvent::process();

    /**
     * Retransmit event
     */
    void transmit();
    void txEventTransmit()
    {
        transmit();
        if (txState == txFifoBlock)
            txKick();
    }
    typedef EventWrapper<Device, &Device::txEventTransmit> TxEvent;
    friend void TxEvent::process();
    TxEvent txEvent;

    void txDump() const;
    void rxDump() const;

    /**
     * receive address filter
     */
    bool rxFilter(const EthPacketPtr &packet);

/**
 * device configuration
 */
    void changeConfig(uint32_t newconfig);
    void command(uint32_t command);

/**
 * device ethernet interface
 */
  public:
    bool recvPacket(EthPacketPtr packet);
    void transferDone();
    virtual EtherInt *getEthPort(const std::string &if_name, int idx);

/**
 * DMA parameters
 */
  protected:
    void rxDmaDone();
    friend class EventWrapper<Device, &Device::rxDmaDone>;
    EventWrapper<Device, &Device::rxDmaDone> rxDmaEvent;

    void txDmaDone();
    friend class EventWrapper<Device, &Device::txDmaDone>;
    EventWrapper<Device, &Device::txDmaDone> txDmaEvent;

    Tick dmaReadDelay;
    Tick dmaReadFactor;
    Tick dmaWriteDelay;
    Tick dmaWriteFactor;

/**
 * Interrupt management
 */
  protected:
    void devIntrPost(uint32_t interrupts);
    void devIntrClear(uint32_t interrupts = Regs::Intr_All);
    void devIntrChangeMask(uint32_t newmask);

/**
 * Memory Interface
 */
  public:
    virtual Tick read(PacketPtr pkt);
    virtual Tick write(PacketPtr pkt);
    virtual void resume();

    void prepareIO(int cpu, int index);
    void prepareRead(int cpu, int index);
    void prepareWrite(int cpu, int index);
 //   Fault iprRead(Addr daddr, int cpu, uint64_t &result);

/**
 * Statistics
 */
  private:
    Stats::Scalar<> rxBytes;
    Stats::Formula  rxBandwidth;
    Stats::Scalar<> rxPackets;
    Stats::Formula  rxPacketRate;
    Stats::Scalar<> rxIpPackets;
    Stats::Scalar<> rxTcpPackets;
    Stats::Scalar<> rxUdpPackets;
    Stats::Scalar<> rxIpChecksums;
    Stats::Scalar<> rxTcpChecksums;
    Stats::Scalar<> rxUdpChecksums;

    Stats::Scalar<> txBytes;
    Stats::Formula  txBandwidth;
    Stats::Formula totBandwidth;
    Stats::Formula totPackets;
    Stats::Formula totBytes;
    Stats::Formula totPacketRate;
    Stats::Scalar<> txPackets;
    Stats::Formula  txPacketRate;
    Stats::Scalar<> txIpPackets;
    Stats::Scalar<> txTcpPackets;
    Stats::Scalar<> txUdpPackets;
    Stats::Scalar<> txIpChecksums;
    Stats::Scalar<> txTcpChecksums;
    Stats::Scalar<> txUdpChecksums;

  public:
    virtual void regStats();

/**
 * Serialization stuff
 */
  public:
    virtual void serialize(std::ostream &os);
    virtual void unserialize(Checkpoint *cp, const std::string &section);

  public:
    Device(const Params *p);
    ~Device();
};

/*
 * Ethernet Interface for an Ethernet Device
 */
class Interface : public EtherInt
{
  private:
    Device *dev;

  public:
    Interface(const std::string &name, Device *d)
        : EtherInt(name), dev(d)
    { }

    virtual bool recvPacket(EthPacketPtr pkt) { return dev->recvPacket(pkt); }
    virtual void sendDone() { dev->transferDone(); }
};

/* namespace Sinic */ }

#endif // __DEV_SINIC_HH__
