/*
 * Copyright (c) 2008 The Regents of The University of Michigan
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors: Ali Saidi
 */

/* @file
 * Device model for Intel's I/O Acceleration Technology (I/OAT).
 * A DMA asyncronous copy engine
 */

#ifndef __DEV_COPY_ENGINE_HH__
#define __DEV_COPY_ENGINE_HH__

#include <vector>

#include "base/statistics.hh"
#include "dev/copy_engine_defs.hh"
#include "dev/pcidev.hh"
#include "params/CopyEngine.hh"
#include "sim/eventq.hh"

class CopyEngine : public PciDev
{
    class CopyEngineChannel
    {
      private:
        DmaPort *cePort;
        CopyEngine *ce;
        CopyEngineReg::ChanRegs  cr;
        int channelId;
        CopyEngineReg::DmaDesc *curDmaDesc;
        uint8_t *copyBuffer;

        bool busy;
        bool underReset;
        bool refreshNext;
        Addr lastDescriptorAddr;
        Addr fetchAddress;

        Tick latBeforeBegin;
        Tick latAfterCompletion;

        uint64_t completionDataReg;

        enum ChannelState {
            Idle,
            AddressFetch,
            DescriptorFetch,
            DMARead,
            DMAWrite,
            CompletionWrite
        };

        ChannelState nextState;

        Event *drainEvent;
      public:
        CopyEngineChannel(CopyEngine *_ce, int cid);
        virtual ~CopyEngineChannel();
        void init();

        std::string name() { assert(ce); return ce->name() + csprintf("-chan%d", channelId); }
        virtual void addressRanges(AddrRangeList &range_list) { range_list.clear(); }
        virtual Tick read(PacketPtr pkt)
                        { panic("CopyEngineChannel has no I/O access\n");}
        virtual Tick write(PacketPtr pkt)
                        { panic("CopyEngineChannel has no I/O access\n"); }

        void channelRead(PacketPtr pkt, Addr daddr, int size);
        void channelWrite(PacketPtr pkt, Addr daddr, int size);

        unsigned int drain(Event *de);
        void resume();
        void serialize(std::ostream &os);
        void unserialize(Checkpoint *cp, const std::string &section);

      private:
        void fetchDescriptor(Addr address);
        void fetchDescComplete();
        EventWrapper<CopyEngineChannel, &CopyEngineChannel::fetchDescComplete>
            fetchCompleteEvent;

        void fetchNextAddr(Addr address);
        void fetchAddrComplete();
        EventWrapper<CopyEngineChannel, &CopyEngineChannel::fetchAddrComplete>
            addrCompleteEvent;

        void readCopyBytes();
        void readCopyBytesComplete();
        EventWrapper<CopyEngineChannel, &CopyEngineChannel::readCopyBytesComplete>
            readCompleteEvent;

        void writeCopyBytes();
        void writeCopyBytesComplete();
        EventWrapper <CopyEngineChannel, &CopyEngineChannel::writeCopyBytesComplete>
            writeCompleteEvent;

        void writeCompletionStatus();
        void writeStatusComplete();
        EventWrapper <CopyEngineChannel, &CopyEngineChannel::writeStatusComplete>
            statusCompleteEvent;


        void continueProcessing();
        void recvCommand();
        bool inDrain();
        void restartStateMachine();
        inline void anBegin(const char *s)
        {
            CPA::cpa()->hwBegin(CPA::FL_NONE, ce->sys,
                         channelId, "CopyEngine", s);
        }

        inline void anWait()
        {
            CPA::cpa()->hwWe(CPA::FL_NONE, ce->sys,
                     channelId, "CopyEngine", "DMAUnusedDescQ", channelId);
        }

        inline void anDq()
        {
            CPA::cpa()->hwDq(CPA::FL_NONE, ce->sys,
                      channelId, "CopyEngine", "DMAUnusedDescQ", channelId);
        }

        inline void anPq()
        {
            CPA::cpa()->hwDq(CPA::FL_NONE, ce->sys,
                      channelId, "CopyEngine", "DMAUnusedDescQ", channelId);
        }

        inline void anQ(const char * s, uint64_t id, int size = 1)
        {
            CPA::cpa()->hwQ(CPA::FL_NONE, ce->sys, channelId,
                    "CopyEngine", s, id, NULL, size);
        }

    };

  private:

    Stats::Vector bytesCopied;
    Stats::Vector copiesProcessed;

    // device registers
    CopyEngineReg::Regs regs;

    // Array of channels each one with regs/dma port/etc
    std::vector<CopyEngineChannel*> chan;

  public:
    typedef CopyEngineParams Params;
    const Params *
    params() const
    {
        return dynamic_cast<const Params *>(_params);
    }
    CopyEngine(const Params *params);
    ~CopyEngine();

    void regStats();
    void init();

    virtual Tick read(PacketPtr pkt);
    virtual Tick write(PacketPtr pkt);

    virtual void serialize(std::ostream &os);
    virtual void unserialize(Checkpoint *cp, const std::string &section);
    virtual unsigned int drain(Event *de);
    virtual void resume();
};

#endif //__DEV_COPY_ENGINE_HH__
