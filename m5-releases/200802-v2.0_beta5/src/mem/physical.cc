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
 *          Ali G. Saidi
 */

#include <sys/types.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <zlib.h>

#include <iostream>
#include <string>

#include "arch/isa_traits.hh"
#include "base/misc.hh"
#include "config/full_system.hh"
#include "mem/packet_access.hh"
#include "mem/physical.hh"
#include "sim/eventq.hh"
#include "sim/host.hh"

using namespace std;
using namespace TheISA;

//
// Hack!  Instead of adding a layer of memory looking like a cache to report
// memory modified during emulation back to HAsim we simply use a few lines
// of code to call back.
//
void (*HAsimNoteMemoryRead)(Addr paddr, uint64_t size) = NULL;
void (*HAsimNoteMemoryWrite)(Addr paddr, uint64_t size) = NULL;


PhysicalMemory::PhysicalMemory(const Params *p)
    : MemObject(p), pmemAddr(NULL), lat(p->latency)
{
    if (params()->range.size() % TheISA::PageBytes != 0)
        panic("Memory Size not divisible by page size\n");

    int map_flags = MAP_ANON | MAP_PRIVATE;
    pmemAddr = (uint8_t *)mmap(NULL, params()->range.size(),
                               PROT_READ | PROT_WRITE, map_flags, -1, 0);

    if (pmemAddr == (void *)MAP_FAILED) {
        perror("mmap");
        fatal("Could not mmap!\n");
    }

    //If requested, initialize all the memory to 0
    if (p->zero)
        memset(pmemAddr, 0, p->range.size());

    pagePtr = 0;

    cachedSize = params()->range.size();
    cachedStart = params()->range.start;

}

void
PhysicalMemory::init()
{
    if (ports.size() == 0) {
        fatal("PhysicalMemory object %s is unconnected!", name());
    }

    for (PortIterator pi = ports.begin(); pi != ports.end(); ++pi) {
        if (*pi)
            (*pi)->sendStatusChange(Port::RangeChange);
    }
}

PhysicalMemory::~PhysicalMemory()
{
    if (pmemAddr)
        munmap((char*)pmemAddr, params()->range.size());
    //Remove memPorts?
}

Addr
PhysicalMemory::new_page()
{
    Addr return_addr = pagePtr << LogVMPageSize;
    return_addr += start();

    ++pagePtr;
    return return_addr;
}

int
PhysicalMemory::deviceBlockSize()
{
    //Can accept anysize request
    return 0;
}

Tick
PhysicalMemory::calculateLatency(PacketPtr pkt)
{
    return lat;
}



// Add load-locked to tracking list.  Should only be called if the
// operation is a load and the LOCKED flag is set.
void
PhysicalMemory::trackLoadLocked(PacketPtr pkt)
{
    Request *req = pkt->req;
    Addr paddr = LockedAddr::mask(req->getPaddr());

    // first we check if we already have a locked addr for this
    // xc.  Since each xc only gets one, we just update the
    // existing record with the new address.
    list<LockedAddr>::iterator i;

    for (i = lockedAddrList.begin(); i != lockedAddrList.end(); ++i) {
        if (i->matchesContext(req)) {
            DPRINTF(LLSC, "Modifying lock record: cpu %d thread %d addr %#x\n",
                    req->getCpuNum(), req->getThreadNum(), paddr);
            i->addr = paddr;
            return;
        }
    }

    // no record for this xc: need to allocate a new one
    DPRINTF(LLSC, "Adding lock record: cpu %d thread %d addr %#x\n",
            req->getCpuNum(), req->getThreadNum(), paddr);
    lockedAddrList.push_front(LockedAddr(req));
}


// Called on *writes* only... both regular stores and
// store-conditional operations.  Check for conventional stores which
// conflict with locked addresses, and for success/failure of store
// conditionals.
bool
PhysicalMemory::checkLockedAddrList(PacketPtr pkt)
{
    Request *req = pkt->req;
    Addr paddr = LockedAddr::mask(req->getPaddr());
    bool isLocked = pkt->isLocked();

    // Initialize return value.  Non-conditional stores always
    // succeed.  Assume conditional stores will fail until proven
    // otherwise.
    bool success = !isLocked;

    // Iterate over list.  Note that there could be multiple matching
    // records, as more than one context could have done a load locked
    // to this location.
    list<LockedAddr>::iterator i = lockedAddrList.begin();

    while (i != lockedAddrList.end()) {

        if (i->addr == paddr) {
            // we have a matching address

            if (isLocked && i->matchesContext(req)) {
                // it's a store conditional, and as far as the memory
                // system can tell, the requesting context's lock is
                // still valid.
                DPRINTF(LLSC, "StCond success: cpu %d thread %d addr %#x\n",
                        req->getCpuNum(), req->getThreadNum(), paddr);
                success = true;
            }

            // Get rid of our record of this lock and advance to next
            DPRINTF(LLSC, "Erasing lock record: cpu %d thread %d addr %#x\n",
                    i->cpuNum, i->threadNum, paddr);
            i = lockedAddrList.erase(i);
        }
        else {
            // no match: advance to next record
            ++i;
        }
    }

    if (isLocked) {
        req->setExtraData(success ? 1 : 0);
    }

    return success;
}


#if TRACING_ON

#define CASE(A, T)                                                      \
  case sizeof(T):                                                       \
    DPRINTF(MemoryAccess, A " of size %i on address 0x%x data 0x%x\n",  \
            pkt->getSize(), pkt->getAddr(), pkt->get<T>());             \
  break


#define TRACE_PACKET(A)                                                 \
    do {                                                                \
        switch (pkt->getSize()) {                                       \
          CASE(A, uint64_t);                                            \
          CASE(A, uint32_t);                                            \
          CASE(A, uint16_t);                                            \
          CASE(A, uint8_t);                                             \
          default:                                                      \
            DPRINTF(MemoryAccess, A " of size %i on address 0x%x\n",    \
                    pkt->getSize(), pkt->getAddr());                    \
        }                                                               \
    } while (0)

#else

#define TRACE_PACKET(A)

#endif

Tick
PhysicalMemory::doAtomicAccess(PacketPtr pkt)
{
    assert(pkt->getAddr() >= start() &&
           pkt->getAddr() + pkt->getSize() <= start() + size());

    if (pkt->memInhibitAsserted()) {
        DPRINTF(MemoryAccess, "mem inhibited on 0x%x: not responding\n",
                pkt->getAddr());
        return 0;
    }

    if (HAsimNoteMemoryWrite != NULL)
    {
        if ((pkt->cmd == MemCmd::SwapReq) || ! pkt->isRead()) {
            HAsimNoteMemoryWrite(pkt->getAddr(), pkt->getSize());
        }
        else {
            HAsimNoteMemoryRead(pkt->getAddr(), pkt->getSize());
        }
    }

    uint8_t *hostAddr = pmemAddr + pkt->getAddr() - start();

    if (pkt->cmd == MemCmd::SwapReq) {
        IntReg overwrite_val;
        bool overwrite_mem;
        uint64_t condition_val64;
        uint32_t condition_val32;

        assert(sizeof(IntReg) >= pkt->getSize());

        overwrite_mem = true;
        // keep a copy of our possible write value, and copy what is at the
        // memory address into the packet
        std::memcpy(&overwrite_val, pkt->getPtr<uint8_t>(), pkt->getSize());
        std::memcpy(pkt->getPtr<uint8_t>(), hostAddr, pkt->getSize());

        if (pkt->req->isCondSwap()) {
            if (pkt->getSize() == sizeof(uint64_t)) {
                condition_val64 = pkt->req->getExtraData();
                overwrite_mem = !std::memcmp(&condition_val64, hostAddr,
                                             sizeof(uint64_t));
            } else if (pkt->getSize() == sizeof(uint32_t)) {
                condition_val32 = (uint32_t)pkt->req->getExtraData();
                overwrite_mem = !std::memcmp(&condition_val32, hostAddr,
                                             sizeof(uint32_t));
            } else
                panic("Invalid size for conditional read/write\n");
        }

        if (overwrite_mem)
            std::memcpy(hostAddr, &overwrite_val, pkt->getSize());

        TRACE_PACKET("Read/Write");
    } else if (pkt->isRead()) {
        assert(!pkt->isWrite());
        if (pkt->isLocked()) {
            trackLoadLocked(pkt);
        }
        memcpy(pkt->getPtr<uint8_t>(), hostAddr, pkt->getSize());
        TRACE_PACKET("Read");
    } else if (pkt->isWrite()) {
        if (writeOK(pkt)) {
            memcpy(hostAddr, pkt->getPtr<uint8_t>(), pkt->getSize());
            TRACE_PACKET("Write");
        }
    } else if (pkt->isInvalidate()) {
        //upgrade or invalidate
        if (pkt->needsResponse()) {
            pkt->makeAtomicResponse();
        }
    } else {
        panic("unimplemented");
    }

    if (pkt->needsResponse()) {
        pkt->makeAtomicResponse();
    }
    return calculateLatency(pkt);
}


void
PhysicalMemory::doFunctionalAccess(PacketPtr pkt)
{
    assert(pkt->getAddr() >= start() &&
           pkt->getAddr() + pkt->getSize() <= start() + size());

    if (HAsimNoteMemoryWrite != NULL)
    {
        if ((pkt->cmd == MemCmd::SwapReq) || ! pkt->isRead()) {
            HAsimNoteMemoryWrite(pkt->getAddr(), pkt->getSize());
        }
        else {
            HAsimNoteMemoryRead(pkt->getAddr(), pkt->getSize());
        }
    }

    uint8_t *hostAddr = pmemAddr + pkt->getAddr() - start();

    if (pkt->isRead()) {
        memcpy(pkt->getPtr<uint8_t>(), hostAddr, pkt->getSize());
        TRACE_PACKET("Read");
        pkt->makeAtomicResponse();
    } else if (pkt->isWrite()) {
        memcpy(hostAddr, pkt->getPtr<uint8_t>(), pkt->getSize());
        TRACE_PACKET("Write");
        pkt->makeAtomicResponse();
    } else if (pkt->isPrint()) {
        Packet::PrintReqState *prs =
            dynamic_cast<Packet::PrintReqState*>(pkt->senderState);
        // Need to call printLabels() explicitly since we're not going
        // through printObj().
        prs->printLabels();
        // Right now we just print the single byte at the specified address.
        ccprintf(prs->os, "%s%#x\n", prs->curPrefix(), *hostAddr);
    } else {
        panic("PhysicalMemory: unimplemented functional command %s",
              pkt->cmdString());
    }
}


Port *
PhysicalMemory::getPort(const std::string &if_name, int idx)
{
    // Accept request for "functional" port for backwards compatibility
    // with places where this function is called from C++.  I'd prefer
    // to move all these into Python someday.
    if (if_name == "functional") {
        return new MemoryPort(csprintf("%s-functional", name()), this);
    }

    if (if_name != "port") {
        panic("PhysicalMemory::getPort: unknown port %s requested", if_name);
    }

    if (idx >= ports.size()) {
        ports.resize(idx+1);
    }

    if (ports[idx] != NULL) {
        panic("PhysicalMemory::getPort: port %d already assigned", idx);
    }

    MemoryPort *port =
        new MemoryPort(csprintf("%s-port%d", name(), idx), this);

    ports[idx] = port;
    return port;
}


void
PhysicalMemory::recvStatusChange(Port::Status status)
{
}

PhysicalMemory::MemoryPort::MemoryPort(const std::string &_name,
                                       PhysicalMemory *_memory)
    : SimpleTimingPort(_name), memory(_memory)
{ }

void
PhysicalMemory::MemoryPort::recvStatusChange(Port::Status status)
{
    memory->recvStatusChange(status);
}

void
PhysicalMemory::MemoryPort::getDeviceAddressRanges(AddrRangeList &resp,
                                                   bool &snoop)
{
    memory->getAddressRanges(resp, snoop);
}

void
PhysicalMemory::getAddressRanges(AddrRangeList &resp, bool &snoop)
{
    snoop = false;
    resp.clear();
    resp.push_back(RangeSize(start(), params()->range.size()));
}

int
PhysicalMemory::MemoryPort::deviceBlockSize()
{
    return memory->deviceBlockSize();
}

Tick
PhysicalMemory::MemoryPort::recvAtomic(PacketPtr pkt)
{
    return memory->doAtomicAccess(pkt);
}

void
PhysicalMemory::MemoryPort::recvFunctional(PacketPtr pkt)
{
    pkt->pushLabel(memory->name());

    if (!checkFunctional(pkt)) {
        // Default implementation of SimpleTimingPort::recvFunctional()
        // calls recvAtomic() and throws away the latency; we can save a
        // little here by just not calculating the latency.
        memory->doFunctionalAccess(pkt);
    }

    pkt->popLabel();
}

unsigned int
PhysicalMemory::drain(Event *de)
{
    int count = 0;
    for (PortIterator pi = ports.begin(); pi != ports.end(); ++pi) {
        count += (*pi)->drain(de);
    }

    if (count)
        changeState(Draining);
    else
        changeState(Drained);
    return count;
}

void
PhysicalMemory::serialize(ostream &os)
{
    gzFile compressedMem;
    string filename = name() + ".physmem";

    SERIALIZE_SCALAR(filename);

    // write memory file
    string thefile = Checkpoint::dir() + "/" + filename.c_str();
    int fd = creat(thefile.c_str(), 0664);
    if (fd < 0) {
        perror("creat");
        fatal("Can't open physical memory checkpoint file '%s'\n", filename);
    }

    compressedMem = gzdopen(fd, "wb");
    if (compressedMem == NULL)
        fatal("Insufficient memory to allocate compression state for %s\n",
                filename);

    if (gzwrite(compressedMem, pmemAddr, params()->range.size()) !=
        params()->range.size()) {
        fatal("Write failed on physical memory checkpoint file '%s'\n",
              filename);
    }

    if (gzclose(compressedMem))
        fatal("Close failed on physical memory checkpoint file '%s'\n",
              filename);
}

void
PhysicalMemory::unserialize(Checkpoint *cp, const string &section)
{
    gzFile compressedMem;
    long *tempPage;
    long *pmem_current;
    uint64_t curSize;
    uint32_t bytesRead;
    const int chunkSize = 16384;


    string filename;

    UNSERIALIZE_SCALAR(filename);

    filename = cp->cptDir + "/" + filename;

    // mmap memoryfile
    int fd = open(filename.c_str(), O_RDONLY);
    if (fd < 0) {
        perror("open");
        fatal("Can't open physical memory checkpoint file '%s'", filename);
    }

    compressedMem = gzdopen(fd, "rb");
    if (compressedMem == NULL)
        fatal("Insufficient memory to allocate compression state for %s\n",
                filename);

    // unmap file that was mmaped in the constructor
    // This is done here to make sure that gzip and open don't muck with our
    // nice large space of memory before we reallocate it
    munmap((char*)pmemAddr, params()->range.size());

    pmemAddr = (uint8_t *)mmap(NULL, params()->range.size(),
        PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);

    if (pmemAddr == (void *)MAP_FAILED) {
        perror("mmap");
        fatal("Could not mmap physical memory!\n");
    }

    curSize = 0;
    tempPage = (long*)malloc(chunkSize);
    if (tempPage == NULL)
        fatal("Unable to malloc memory to read file %s\n", filename);

    /* Only copy bytes that are non-zero, so we don't give the VM system hell */
    while (curSize < params()->range.size()) {
        bytesRead = gzread(compressedMem, tempPage, chunkSize);
        if (bytesRead != chunkSize &&
            bytesRead != params()->range.size() - curSize)
            fatal("Read failed on physical memory checkpoint file '%s'"
                  " got %d bytes, expected %d or %d bytes\n",
                  filename, bytesRead, chunkSize,
                  params()->range.size() - curSize);

        assert(bytesRead % sizeof(long) == 0);

        for (int x = 0; x < bytesRead/sizeof(long); x++)
        {
             if (*(tempPage+x) != 0) {
                 pmem_current = (long*)(pmemAddr + curSize + x * sizeof(long));
                 *pmem_current = *(tempPage+x);
             }
        }
        curSize += bytesRead;
    }

    free(tempPage);

    if (gzclose(compressedMem))
        fatal("Close failed on physical memory checkpoint file '%s'\n",
              filename);

}

PhysicalMemory *
PhysicalMemoryParams::create()
{
    return new PhysicalMemory(this);
}