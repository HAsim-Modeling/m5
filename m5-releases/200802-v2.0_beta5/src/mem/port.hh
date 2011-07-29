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
 */

/**
 * @file
 * Port Object Declaration. Ports are used to interface memory objects to
 * each other.  They will always come in pairs, and we refer to the other
 * port object as the peer.  These are used to make the design more
 * modular so that a specific interface between every type of objcet doesn't
 * have to be created.
 */

#ifndef __MEM_PORT_HH__
#define __MEM_PORT_HH__

#include <list>
#include <inttypes.h>

#include "base/misc.hh"
#include "base/range.hh"
#include "mem/packet.hh"
#include "mem/request.hh"

/** This typedef is used to clean up the parameter list of
 * getDeviceAddressRanges() and getPeerAddressRanges().  It's declared
 * outside the Port object since it's also used by some mem objects.
 * Eventually we should move this typedef to wherever Addr is
 * defined.
 */

typedef std::list<Range<Addr> > AddrRangeList;
typedef std::list<Range<Addr> >::iterator AddrRangeIter;

class MemObject;

/**
 * Ports are used to interface memory objects to
 * each other.  They will always come in pairs, and we refer to the other
 * port object as the peer.  These are used to make the design more
 * modular so that a specific interface between every type of objcet doesn't
 * have to be created.
 *
 * Recv accesor functions are being called from the peer interface.
 * Send accessor functions are being called from the device the port is
 * associated with, and it will call the peer recv. accessor function.
 */
class Port
{
  private:

    /** Descriptive name (for DPRINTF output) */
    mutable std::string portName;

    /** A pointer to the peer port.  Ports always come in pairs, that way they
        can use a standardized interface to communicate between different
        memory objects. */
    Port *peer;

    /** A pointer to the MemObject that owns this port. This may not be set. */
    MemObject *owner;

  public:

    Port();

    /**
     * Constructor.
     *
     * @param _name Port name for DPRINTF output.  Should include name
     * of memory system object to which the port belongs.
     * @param _owner Pointer to the MemObject that owns this port.
     * Will not necessarily be set.
     */
    Port(const std::string &_name, MemObject *_owner = NULL);

    /** Return port name (for DPRINTF). */
    const std::string &name() const { return portName; }

    virtual ~Port() {};

    // mey be better to use subclasses & RTTI?
    /** Holds the ports status.  Currently just that a range recomputation needs
     * to be done. */
    enum Status {
        RangeChange
    };

    void setName(const std::string &name)
    { portName = name; }

    /** Function to set the pointer for the peer port. */
    virtual void setPeer(Port *port);

    /** Function to get the pointer to the peer port. */
    Port *getPeer() { return peer; }

    /** Function to set the owner of this port. */
    void setOwner(MemObject *_owner) { owner = _owner; }

    /** Function to return the owner of this port. */
    MemObject *getOwner() { return owner; }

    /** Inform the peer port to delete itself and notify it's owner about it's
     * demise. */
    void removeConn();

    virtual bool isDefaultPort() { return false; }

  protected:

    /** These functions are protected because they should only be
     * called by a peer port, never directly by any outside object. */

    /** Called to recive a timing call from the peer port. */
    virtual bool recvTiming(PacketPtr pkt) = 0;

    /** Called to recive a atomic call from the peer port. */
    virtual Tick recvAtomic(PacketPtr pkt) = 0;

    /** Called to recive a functional call from the peer port. */
    virtual void recvFunctional(PacketPtr pkt) = 0;

    /** Called to recieve a status change from the peer port. */
    virtual void recvStatusChange(Status status) = 0;

    /** Called by a peer port if the send was unsuccesful, and had to
        wait.  This shouldn't be valid for response paths (IO Devices).
        so it is set to panic if it isn't already defined.
    */
    virtual void recvRetry() { panic("??"); }

    /** Called by a peer port in order to determine the block size of the
        device connected to this port.  It sometimes doesn't make sense for
        this function to be called, so it just returns 0. Anytthing that is
        concerned with the size should just ignore that.
    */
    virtual int deviceBlockSize() { return 0; }

    /** The peer port is requesting us to reply with a list of the ranges we
        are responsible for.
        @param resp is a list of ranges responded to
        @param snoop is a list of ranges snooped
    */
    virtual void getDeviceAddressRanges(AddrRangeList &resp,
                                        bool &snoop)
    { panic("??"); }

  public:

    /** Function called by associated memory device (cache, memory, iodevice)
        in order to send a timing request to the port.  Simply calls the peer
        port receive function.
        @return This function returns if the send was succesful in it's
        recieve. If it was a failure, then the port will wait for a recvRetry
        at which point it can possibly issue a successful sendTiming.  This is used in
        case a cache has a higher priority request come in while waiting for
        the bus to arbitrate.
    */
    bool sendTiming(PacketPtr pkt) { return peer->recvTiming(pkt); }

    /** Function called by the associated device to send an atomic
     *   access, an access in which the data is moved and the state is
     *   updated in one cycle, without interleaving with other memory
     *   accesses.  Returns estimated latency of access.
     */
    Tick sendAtomic(PacketPtr pkt)
        { return peer->recvAtomic(pkt); }

    /** Function called by the associated device to send a functional access,
        an access in which the data is instantly updated everywhere in the
        memory system, without affecting the current state of any block or
        moving the block.
    */
    void sendFunctional(PacketPtr pkt)
        { return peer->recvFunctional(pkt); }

    /** Called by the associated device to send a status change to the device
        connected to the peer interface.
    */
    void sendStatusChange(Status status) {peer->recvStatusChange(status); }

    /** When a timing access doesn't return a success, some time later the
        Retry will be sent.
    */
    void sendRetry() { return peer->recvRetry(); }

    /** Called by the associated device if it wishes to find out the blocksize
        of the device on attached to the peer port.
    */
    int peerBlockSize() { return peer->deviceBlockSize(); }

    /** Called by the associated device if it wishes to find out the address
        ranges connected to the peer ports devices.
    */
    void getPeerAddressRanges(AddrRangeList &resp, bool &snoop)
    { peer->getDeviceAddressRanges(resp, snoop); }

    /** This function is a wrapper around sendFunctional()
        that breaks a larger, arbitrarily aligned access into
        appropriate chunks.  The default implementation can use
        getBlockSize() to determine the block size and go from there.
    */
    virtual void readBlob(Addr addr, uint8_t *p, int size);

    /** This function is a wrapper around sendFunctional()
        that breaks a larger, arbitrarily aligned access into
        appropriate chunks.  The default implementation can use
        getBlockSize() to determine the block size and go from there.
    */
    virtual void writeBlob(Addr addr, uint8_t *p, int size);

    /** Fill size bytes starting at addr with byte value val.  This
        should not need to be virtual, since it can be implemented in
        terms of writeBlob().  However, it shouldn't be
        performance-critical either, so it could be if we wanted to.
    */
    virtual void memsetBlob(Addr addr, uint8_t val, int size);

    /** Inject a PrintReq for the given address to print the state of
     * that address throughout the memory system.  For debugging.
     */
    void printAddr(Addr a);

  private:

    /** Internal helper function for read/writeBlob().
     */
    void blobHelper(Addr addr, uint8_t *p, int size, MemCmd cmd);
};

/** A simple functional port that is only meant for one way communication to
 * physical memory. It is only meant to be used to load data into memory before
 * the simulation begins.
 */

class FunctionalPort : public Port
{
  public:
    FunctionalPort(const std::string &_name, MemObject *_owner = NULL)
        : Port(_name, _owner)
    {}

  protected:
    virtual bool recvTiming(PacketPtr pkt) { panic("FuncPort is UniDir");
        M5_DUMMY_RETURN }
    virtual Tick recvAtomic(PacketPtr pkt) { panic("FuncPort is UniDir");
        M5_DUMMY_RETURN }
    virtual void recvFunctional(PacketPtr pkt) { panic("FuncPort is UniDir"); }
    virtual void recvStatusChange(Status status) {}

  public:
    /** a write function that also does an endian conversion. */
    template <typename T>
    inline void writeHtoG(Addr addr, T d);

    /** a read function that also does an endian conversion. */
    template <typename T>
    inline T readGtoH(Addr addr);

    template <typename T>
    inline void write(Addr addr, T d)
    {
        writeBlob(addr, (uint8_t*)&d, sizeof(T));
    }

    template <typename T>
    inline T read(Addr addr)
    {
        T d;
        readBlob(addr, (uint8_t*)&d, sizeof(T));
        return d;
    }
};

#endif //__MEM_PORT_HH__