/*
 * Copyright (c) 2001, 2002, 2003, 2004, 2005
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
 * Authors: Ronald G. Dreslinski Jr
 */

/* @file
 */

#ifndef __PHYSICAL_MEMORY_HH__
#define __PHYSICAL_MEMORY_HH__

#include <map>
#include <string>

#include "base/range.hh"
#include "mem/mem_object.hh"
#include "mem/packet.hh"
#include "mem/tport.hh"
#include "params/PhysicalMemory.hh"
#include "sim/eventq.hh"

//
// Functional model for a contiguous block of physical memory. (i.e. RAM)
//
class PhysicalMemory : public MemObject
{
    class MemoryPort : public SimpleTimingPort
    {
        PhysicalMemory *memory;

      public:

        MemoryPort(const std::string &_name, PhysicalMemory *_memory);

      protected:

        virtual Tick recvAtomic(PacketPtr pkt);

        virtual void recvFunctional(PacketPtr pkt);

        virtual void recvStatusChange(Status status);

        virtual void getDeviceAddressRanges(AddrRangeList &resp,
                                            bool &snoop);

        virtual int deviceBlockSize();
    };

    int numPorts;


  private:
    // prevent copying of a MainMemory object
    PhysicalMemory(const PhysicalMemory &specmem);
    const PhysicalMemory &operator=(const PhysicalMemory &specmem);

  protected:

    class LockedAddr {
      public:
        // on alpha, minimum LL/SC granularity is 16 bytes, so lower
        // bits need to masked off.
        static const Addr Addr_Mask = 0xf;

        static Addr mask(Addr paddr) { return (paddr & ~Addr_Mask); }

        Addr addr; 	// locked address
        int cpuNum;	// locking CPU
        int threadNum;	// locking thread ID within CPU

        // check for matching execution context
        bool matchesContext(Request *req)
        {
            return (cpuNum == req->getCpuNum() &&
                    threadNum == req->getThreadNum());
        }

        LockedAddr(Request *req)
            : addr(mask(req->getPaddr())),
              cpuNum(req->getCpuNum()),
              threadNum(req->getThreadNum())
        {
        }
    };

    std::list<LockedAddr> lockedAddrList;

    // helper function for checkLockedAddrs(): we really want to
    // inline a quick check for an empty locked addr list (hopefully
    // the common case), and do the full list search (if necessary) in
    // this out-of-line function
    bool checkLockedAddrList(PacketPtr pkt);

    // Record the address of a load-locked operation so that we can
    // clear the execution context's lock flag if a matching store is
    // performed
    void trackLoadLocked(PacketPtr pkt);

    // Compare a store address with any locked addresses so we can
    // clear the lock flag appropriately.  Return value set to 'false'
    // if store operation should be suppressed (because it was a
    // conditional store and the address was no longer locked by the
    // requesting execution context), 'true' otherwise.  Note that
    // this method must be called on *all* stores since even
    // non-conditional stores must clear any matching lock addresses.
    bool writeOK(PacketPtr pkt) {
        Request *req = pkt->req;
        if (lockedAddrList.empty()) {
            // no locked addrs: nothing to check, store_conditional fails
            bool isLocked = pkt->isLocked();
            if (isLocked) {
                req->setExtraData(0);
            }
            return !isLocked; // only do write if not an sc
        } else {
            // iterate over list...
            return checkLockedAddrList(pkt);
        }
    }

    uint8_t *pmemAddr;
    int pagePtr;
    Tick lat;
    std::vector<MemoryPort*> ports;
    typedef std::vector<MemoryPort*>::iterator PortIterator;

    uint64_t cachedSize;
    uint64_t cachedStart;
  public:
    Addr new_page();
    uint64_t size() { return cachedSize; }
    uint64_t start() { return cachedStart; }

  public:
    typedef PhysicalMemoryParams Params;
    PhysicalMemory(const Params *p);
    virtual ~PhysicalMemory();

    const Params *
    params() const
    {
        return dynamic_cast<const Params *>(_params);
    }

  public:
    int deviceBlockSize();
    void getAddressRanges(AddrRangeList &resp, bool &snoop);
    virtual Port *getPort(const std::string &if_name, int idx = -1);
    void virtual init();
    unsigned int drain(Event *de);

  protected:
    Tick doAtomicAccess(PacketPtr pkt);
    void doFunctionalAccess(PacketPtr pkt);
    virtual Tick calculateLatency(PacketPtr pkt);
    void recvStatusChange(Port::Status status);

  public:
    virtual void serialize(std::ostream &os);
    virtual void unserialize(Checkpoint *cp, const std::string &section);

};

#endif //__PHYSICAL_MEMORY_HH__
