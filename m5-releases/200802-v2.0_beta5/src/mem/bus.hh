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
 * Authors: Ronald G. Dreslinski Jr
 *          Ali G. Saidi
 */

/**
 * @file
 * Declaration of a bus object.
 */

#ifndef __MEM_BUS_HH__
#define __MEM_BUS_HH__

#include <string>
#include <set>
#include <list>
#include <inttypes.h>

#include "base/range.hh"
#include "base/hashmap.hh"
#include "base/range_map.hh"
#include "mem/mem_object.hh"
#include "mem/packet.hh"
#include "mem/port.hh"
#include "mem/request.hh"
#include "sim/eventq.hh"
#include "params/Bus.hh"

class Bus : public MemObject
{
    /** Declaration of the buses port type, one will be instantiated for each
        of the interfaces connecting to the bus. */
    class BusPort : public Port
    {
        bool _onRetryList;

        /** A pointer to the bus to which this port belongs. */
        Bus *bus;

        /** A id to keep track of the intercafe ID this port is connected to. */
        int id;

      public:

        /** Constructor for the BusPort.*/
        BusPort(const std::string &_name, Bus *_bus, int _id)
            : Port(_name, _bus), _onRetryList(false), bus(_bus), id(_id)
        { }

        bool onRetryList()
        { return _onRetryList; }

        void onRetryList(bool newVal)
        { _onRetryList = newVal; }

        int getId() { return id; }

      protected:

        /** When reciving a timing request from the peer port (at id),
            pass it to the bus. */
        virtual bool recvTiming(PacketPtr pkt)
        { pkt->setSrc(id); return bus->recvTiming(pkt); }

        /** When reciving a Atomic requestfrom the peer port (at id),
            pass it to the bus. */
        virtual Tick recvAtomic(PacketPtr pkt)
        { pkt->setSrc(id); return bus->recvAtomic(pkt); }

        /** When reciving a Functional requestfrom the peer port (at id),
            pass it to the bus. */
        virtual void recvFunctional(PacketPtr pkt)
        { pkt->setSrc(id); bus->recvFunctional(pkt); }

        /** When reciving a status changefrom the peer port (at id),
            pass it to the bus. */
        virtual void recvStatusChange(Status status)
        { bus->recvStatusChange(status, id); }

        /** When reciving a retry from the peer port (at id),
            pass it to the bus. */
        virtual void recvRetry()
        { bus->recvRetry(id); }

        // This should return all the 'owned' addresses that are
        // downstream from this bus, yes?  That is, the union of all
        // the 'owned' address ranges of all the other interfaces on
        // this bus...
        virtual void getDeviceAddressRanges(AddrRangeList &resp,
                                            bool &snoop)
        { bus->addressRanges(resp, snoop, id); }

        // Ask the bus to ask everyone on the bus what their block size is and
        // take the max of it. This might need to be changed a bit if we ever
        // support multiple block sizes.
        virtual int deviceBlockSize()
        { return bus->findBlockSize(id); }

    };

    class BusFreeEvent : public Event
    {
        Bus * bus;

      public:
        BusFreeEvent(Bus * _bus);
        void process();
        const char *description() const;
    };

    /** a globally unique id for this bus. */
    int busId;
    /** the clock speed for the bus */
    int clock;
    /** cycles of overhead per transaction */
    int headerCycles;
    /** the width of the bus in bytes */
    int width;
    /** the next tick at which the bus will be idle */
    Tick tickNextIdle;

    Event * drainEvent;


    static const int defaultId = -3; //Make it unique from Broadcast

    typedef range_map<Addr,int>::iterator PortIter;
    range_map<Addr, int> portMap;

    AddrRangeList defaultRange;

    typedef std::vector<BusPort*>::iterator SnoopIter;
    std::vector<BusPort*> snoopPorts;

    /** Function called by the port when the bus is recieving a Timing
      transaction.*/
    bool recvTiming(PacketPtr pkt);

    /** Function called by the port when the bus is recieving a Atomic
      transaction.*/
    Tick recvAtomic(PacketPtr pkt);

    /** Function called by the port when the bus is recieving a Functional
        transaction.*/
    void recvFunctional(PacketPtr pkt);

    /** Timing function called by port when it is once again able to process
     * requests. */
    void recvRetry(int id);

    /** Function called by the port when the bus is recieving a status change.*/
    void recvStatusChange(Port::Status status, int id);

    /** Find which port connected to this bus (if any) should be given a packet
     * with this address.
     * @param addr Address to find port for.
     * @return id of port that the packet should be sent out of.
     */
    int findPort(Addr addr);

    // Cache for the findPort function storing recently used ports from portMap
    struct PortCache {
        bool valid;
        int  id;
        Addr start;
        Addr end;
    };

    PortCache portCache[3];
    
    // Checks the cache and returns the id of the port that has the requested 
    // address within its range
    inline int checkPortCache(Addr addr) {
        if (portCache[0].valid && addr >= portCache[0].start && 
            addr < portCache[0].end) {
            return portCache[0].id;
        }
        if (portCache[1].valid && addr >= portCache[1].start && 
                   addr < portCache[1].end) {
            return portCache[1].id;
        }
        if (portCache[2].valid && addr >= portCache[2].start && 
            addr < portCache[2].end) {
            return portCache[2].id;
        }
        
        return -1;
    }

    // Clears the earliest entry of the cache and inserts a new port entry
    inline void updatePortCache(short id, Addr start, Addr end) {
        portCache[2].valid = portCache[1].valid;
        portCache[2].id    = portCache[1].id;
        portCache[2].start = portCache[1].start;
        portCache[2].end   = portCache[1].end;

        portCache[1].valid = portCache[0].valid;
        portCache[1].id    = portCache[0].id;
        portCache[1].start = portCache[0].start;
        portCache[1].end   = portCache[0].end;

        portCache[0].valid = true;
        portCache[0].id    = id;
        portCache[0].start = start;
        portCache[0].end   = end;
    }
  
    // Clears the cache. Needs to be called in constructor.
    inline void clearPortCache() {
        portCache[2].valid = false;
        portCache[1].valid = false;
        portCache[0].valid = false;
    }    

    /** Process address range request.
     * @param resp addresses that we can respond to
     * @param snoop addresses that we would like to snoop
     * @param id ide of the busport that made the request.
     */
    void addressRanges(AddrRangeList &resp, bool &snoop, int id);

    /** Prepare a packet to be sent on the bus. The header finishes at tick
     *  headerTime
     */
    void preparePacket(PacketPtr pkt, Tick & headerTime);

    /** Occupy the bus until until */
    void occupyBus(Tick until);

    /** Ask everyone on the bus what their size is
     * @param id id of the busport that made the request
     * @return the max of all the sizes
     */
    int findBlockSize(int id);

    BusFreeEvent busIdle;

    bool inRetry;
    std::set<int> inRecvStatusChange;

    /** max number of bus ids we've handed out so far */
    short maxId;

    /** An array of pointers to the peer port interfaces
        connected to this bus.*/
    m5::hash_map<short,BusPort*> interfaces;

    /** An array of pointers to ports that retry should be called on because the
     * original send failed for whatever reason.*/
    std::list<BusPort*> retryList;

    void addToRetryList(BusPort * port)
    {
        if (!inRetry) {
            // The device wasn't retrying a packet, or wasn't at an appropriate
            // time.
            assert(!port->onRetryList());
            port->onRetryList(true);
            retryList.push_back(port);
        } else {
            if (port->onRetryList()) {
                // The device was retrying a packet. It didn't work, so we'll leave
                // it at the head of the retry list.
                assert(port == retryList.front());
                inRetry = false;
            }
            else {
                port->onRetryList(true);
                retryList.push_back(port);
            }
        }
    }

    /** Port that handles requests that don't match any of the interfaces.*/
    BusPort *defaultPort;

    BusPort *funcPort;
    int funcPortId;

    /** Has the user specified their own default responder? */
    bool responderSet;

    int defaultBlockSize;
    int cachedBlockSize;
    bool cachedBlockSizeValid;

   // Cache for the peer port interfaces 
    struct BusCache {
        bool  valid;
        short id;
        BusPort  *port;
    };
 
    BusCache busCache[3];
    
    // Checks the peer port interfaces cache for the port id and returns
    // a pointer to the matching port
    inline BusPort* checkBusCache(short id) {
        if (busCache[0].valid && id == busCache[0].id) {
            return busCache[0].port;
        }
        if (busCache[1].valid && id == busCache[1].id) {
            return busCache[1].port;
        }
        if (busCache[2].valid && id == busCache[2].id) {
            return busCache[2].port;
        }
        
        return NULL;
    }

    // Replaces the earliest entry in the cache with a new entry
    inline void updateBusCache(short id, BusPort *port) {
        busCache[2].valid = busCache[1].valid;
        busCache[2].id    = busCache[1].id;
        busCache[2].port  = busCache[1].port;

        busCache[1].valid = busCache[0].valid;
        busCache[1].id    = busCache[0].id;
        busCache[1].port  = busCache[0].port;

        busCache[0].valid = true;
        busCache[0].id    = id;
        busCache[0].port  = port;
    }

    // Invalidates the cache. Needs to be called in constructor.
    inline void clearBusCache() {
        busCache[2].valid = false;
        busCache[1].valid = false;	
        busCache[0].valid = false;
    }


  public:

    /** A function used to return the port associated with this bus object. */
    virtual Port *getPort(const std::string &if_name, int idx = -1);
    virtual void deletePortRefs(Port *p);

    virtual void init();
    virtual void startup();

    unsigned int drain(Event *de);

    Bus(const BusParams *p)
        : MemObject(p), busId(p->bus_id), clock(p->clock),
          headerCycles(p->header_cycles), width(p->width), tickNextIdle(0),
          drainEvent(NULL), busIdle(this), inRetry(false), maxId(0),
          defaultPort(NULL), funcPort(NULL), funcPortId(-4),
          responderSet(p->responder_set), defaultBlockSize(p->block_size),
          cachedBlockSize(0), cachedBlockSizeValid(false)
    {
        //width, clock period, and header cycles must be positive
        if (width <= 0)
            fatal("Bus width must be positive\n");
        if (clock <= 0)
            fatal("Bus clock period must be positive\n");
        if (headerCycles <= 0)
            fatal("Number of header cycles must be positive\n");
        clearBusCache();
        clearPortCache();
    }

};

#endif //__MEM_BUS_HH__