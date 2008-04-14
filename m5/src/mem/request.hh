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
 *          Steven K. Reinhardt
 *          Ali G. Saidi
 */

/**
 * @file
 * Declaration of a request, the overall memory request consisting of
 the parts of the request that are persistent throughout the transaction.
 */

#ifndef __MEM_REQUEST_HH__
#define __MEM_REQUEST_HH__

#include "base/fast_alloc.hh"
#include "sim/host.hh"
#include "sim/core.hh"

#include <cassert>

class Request;

typedef Request* RequestPtr;


/** ASI information for this request if it exsits. */
const uint32_t ASI_BITS         = 0x000FF;
/** The request is a Load locked/store conditional. */
const uint32_t LOCKED		= 0x00100;
/** The virtual address is also the physical address. */
const uint32_t PHYSICAL		= 0x00200;
/** The request is an ALPHA VPTE pal access (hw_ld). */
const uint32_t VPTE		= 0x00400;
/** Use the alternate mode bits in ALPHA. */
const uint32_t ALTMODE		= 0x00800;
/** The request is to an uncacheable address. */
const uint32_t UNCACHEABLE	= 0x01000;
/** The request should not cause a page fault. */
const uint32_t NO_FAULT         = 0x02000;
/** The request should be prefetched into the exclusive state. */
const uint32_t PF_EXCLUSIVE	= 0x10000;
/** The request should be marked as LRU. */
const uint32_t EVICT_NEXT	= 0x20000;
/** The request should ignore unaligned access faults */
const uint32_t NO_ALIGN_FAULT   = 0x40000;
/** The request was an instruction read. */
const uint32_t INST_READ        = 0x80000;
/** This request is for a memory swap. */
const uint32_t MEM_SWAP         = 0x100000;
const uint32_t MEM_SWAP_COND    = 0x200000;
/** The request should ignore unaligned access faults */
const uint32_t NO_HALF_WORD_ALIGN_FAULT = 0x400000;


class Request : public FastAlloc
{
  private:
    /**
     * The physical address of the request. Valid only if validPaddr
     * is set. */
    Addr paddr;

    /**
     * The size of the request. This field must be set when vaddr or
     * paddr is written via setVirt() or setPhys(), so it is always
     * valid as long as one of the address fields is valid.  */
    int size;

    /** Flag structure for the request. */
    uint32_t flags;

    /**
     * The time this request was started. Used to calculate
     * latencies. This field is set to curTick any time paddr or vaddr
     * is written.  */
    Tick time;

    /** The address space ID. */
    int asid;

    /** This request is to a memory mapped register. */
    bool mmapedIpr;

    /** The virtual address of the request. */
    Addr vaddr;

    /** Extra data for the request, such as the return value of
     * store conditional or the compare value for a CAS. */
    uint64_t extraData;

    /** The cpu number (for statistics, typically). */
    int cpuNum;
    /** The requesting thread id (for statistics, typically). */
    int  threadNum;

    /** program counter of initiating access; for tracing/debugging */
    Addr pc;

    /** Whether or not paddr is valid (has been written yet). */
    bool validPaddr;
    /** Whether or not the asid & vaddr are valid. */
    bool validAsidVaddr;
    /** Whether or not the sc result is valid. */
    bool validExData;
    /** Whether or not the cpu number & thread ID are valid. */
    bool validCpuAndThreadNums;
    /** Whether or not the pc is valid. */
    bool validPC;

  public:
    /** Minimal constructor.  No fields are initialized. */
    Request()
        : validPaddr(false), validAsidVaddr(false),
          validExData(false), validCpuAndThreadNums(false), validPC(false)
    {}

    /**
     * Constructor for physical (e.g. device) requests.  Initializes
     * just physical address, size, flags, and timestamp (to curTick).
     * These fields are adequate to perform a request.  */
    Request(Addr _paddr, int _size, int _flags)
        : validCpuAndThreadNums(false)
    { setPhys(_paddr, _size, _flags); }

    Request(int _asid, Addr _vaddr, int _size, int _flags, Addr _pc,
            int _cpuNum, int _threadNum)
    {
        setThreadContext(_cpuNum, _threadNum);
        setVirt(_asid, _vaddr, _size, _flags, _pc);
    }

    ~Request() {}  // for FastAlloc

    /**
     * Set up CPU and thread numbers. */
    void setThreadContext(int _cpuNum, int _threadNum)
    {
        cpuNum = _cpuNum;
        threadNum = _threadNum;
        validCpuAndThreadNums = true;
    }

    /**
     * Set up a physical (e.g. device) request in a previously
     * allocated Request object. */
    void setPhys(Addr _paddr, int _size, int _flags)
    {
        paddr = _paddr;
        size = _size;
        flags = _flags;
        time = curTick;
        validPaddr = true;
        validAsidVaddr = false;
        validPC = false;
        validExData = false;
        mmapedIpr = false;
    }

    /**
     * Set up a virtual (e.g., CPU) request in a previously
     * allocated Request object. */
    void setVirt(int _asid, Addr _vaddr, int _size, int _flags, Addr _pc)
    {
        asid = _asid;
        vaddr = _vaddr;
        size = _size;
        flags = _flags;
        pc = _pc;
        time = curTick;
        validPaddr = false;
        validAsidVaddr = true;
        validPC = true;
        validExData = false;
        mmapedIpr = false;
    }

    /** Set just the physical address.  This should only be used to
     * record the result of a translation, and thus the vaddr must be
     * valid before this method is called.  Otherwise, use setPhys()
     * to guarantee that the size and flags are also set.
     */
    void setPaddr(Addr _paddr)
    {
        assert(validAsidVaddr);
        paddr = _paddr;
        validPaddr = true;
    }

    /** Accessor for paddr. */
    Addr getPaddr() { assert(validPaddr); return paddr; }

    /** Accessor for size. */
    int getSize() { assert(validPaddr || validAsidVaddr); return size; }
    /** Accessor for time. */
    Tick getTime() { assert(validPaddr || validAsidVaddr); return time; }

    /** Accessor for flags. */
    uint32_t getFlags() { assert(validPaddr || validAsidVaddr); return flags; }
    /** Accessor for paddr. */
    void setFlags(uint32_t _flags)
    { assert(validPaddr || validAsidVaddr); flags = _flags; }

    /** Accessor function for vaddr.*/
    Addr getVaddr() { assert(validAsidVaddr); return vaddr; }

    /** Accessor function for asid.*/
    int getAsid() { assert(validAsidVaddr); return asid; }

    /** Accessor function for asi.*/
    uint8_t getAsi() { assert(validAsidVaddr); return flags & ASI_BITS; }

    /** Accessor function for asi.*/
    void setAsi(uint8_t a)
    { assert(validAsidVaddr); flags = (flags & ~ASI_BITS) | a; }

    /** Accessor function for asi.*/
    bool isMmapedIpr() { assert(validPaddr); return mmapedIpr; }

    /** Accessor function for asi.*/
    void setMmapedIpr(bool r) { assert(validAsidVaddr); mmapedIpr = r; }

    /** Accessor function to check if sc result is valid. */
    bool extraDataValid() { return validExData; }
    /** Accessor function for store conditional return value.*/
    uint64_t getExtraData() { assert(validExData); return extraData; }
    /** Accessor function for store conditional return value.*/
    void setExtraData(uint64_t _extraData)
    { extraData = _extraData; validExData = true; }

    /** Accessor function for cpu number.*/
    int getCpuNum() { assert(validCpuAndThreadNums); return cpuNum; }
    /** Accessor function for thread number.*/
    int getThreadNum()  { assert(validCpuAndThreadNums); return threadNum; }

    /** Accessor function for pc.*/
    Addr getPC() { assert(validPC); return pc; }

    /** Accessor Function to Check Cacheability. */
    bool isUncacheable() { return (getFlags() & UNCACHEABLE) != 0; }

    bool isInstRead() { return (getFlags() & INST_READ) != 0; }

    bool isLocked() { return (getFlags() & LOCKED) != 0; }

    bool isSwap() { return (getFlags() & MEM_SWAP ||
                            getFlags() & MEM_SWAP_COND); }

    bool isCondSwap() { return (getFlags() & MEM_SWAP_COND) != 0; }

    bool inline isMisaligned() {return (!(getFlags() & NO_ALIGN_FAULT) && 
					((vaddr & 1)  || 
					 (!(getFlags() & NO_HALF_WORD_ALIGN_FAULT) 
					  && (vaddr & 0x2))));}

    friend class Packet;
};

#endif // __MEM_REQUEST_HH__
