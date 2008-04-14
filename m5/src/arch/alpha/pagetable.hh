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
 * Authors: Nathan L. Binkert
 *          Steven K. Reinhardt
 */

#ifndef __ARCH_ALPHA_PAGETABLE_H__
#define __ARCH_ALPHA_PAGETABLE_H__

#include "arch/alpha/isa_traits.hh"
#include "arch/alpha/utility.hh"
#include "config/full_system.hh"

namespace AlphaISA {

    struct VAddr
    {
        static const int ImplBits = 43;
        static const Addr ImplMask = (ULL(1) << ImplBits) - 1;
        static const Addr UnImplMask = ~ImplMask;

        VAddr(Addr a) : addr(a) {}
        Addr addr;
        operator Addr() const { return addr; }
        const VAddr &operator=(Addr a) { addr = a; return *this; }

        Addr vpn() const { return (addr & ImplMask) >> PageShift; }
        Addr page() const { return addr & PageMask; }
        Addr offset() const { return addr & PageOffset; }

        Addr level3() const
        { return AlphaISA::PteAddr(addr >> PageShift); }
        Addr level2() const
        { return AlphaISA::PteAddr(addr >> NPtePageShift + PageShift); }
        Addr level1() const
        { return AlphaISA::PteAddr(addr >> 2 * NPtePageShift + PageShift); }
    };

    struct PageTableEntry
    {
        PageTableEntry(uint64_t e) : entry(e) {}
        uint64_t entry;
        operator uint64_t() const { return entry; }
        const PageTableEntry &operator=(uint64_t e) { entry = e; return *this; }
        const PageTableEntry &operator=(const PageTableEntry &e)
        { entry = e.entry; return *this; }

        Addr _pfn()  const { return (entry >> 32) & 0xffffffff; }
        Addr _sw()   const { return (entry >> 16) & 0xffff; }
        int  _rsv0() const { return (entry >> 14) & 0x3; }
        bool _uwe()  const { return (entry >> 13) & 0x1; }
        bool _kwe()  const { return (entry >> 12) & 0x1; }
        int  _rsv1() const { return (entry >> 10) & 0x3; }
        bool _ure()  const { return (entry >>  9) & 0x1; }
        bool _kre()  const { return (entry >>  8) & 0x1; }
        bool _nomb() const { return (entry >>  7) & 0x1; }
        int  _gh()   const { return (entry >>  5) & 0x3; }
        bool _asm_()  const { return (entry >>  4) & 0x1; }
        bool _foe()  const { return (entry >>  3) & 0x1; }
        bool _fow()  const { return (entry >>  2) & 0x1; }
        bool _for()  const { return (entry >>  1) & 0x1; }
        bool valid() const { return (entry >>  0) & 0x1; }

        Addr paddr() const { return _pfn() << PageShift; }
    };

    // ITB/DTB table entry
    struct TlbEntry
    {
        //Construct an entry that maps to physical address addr.
        TlbEntry(Addr _asn, Addr _vaddr, Addr _paddr)
        {
            VAddr vaddr(_vaddr);
            VAddr paddr(_paddr);
            tag = vaddr.vpn();
            ppn = paddr.vpn();
            xre = 15;
            xwe = 15;
            asn = _asn;
            asma = false;
            fonr = false;
            fonw = false;
            valid = true;
        }
        TlbEntry()
        {}

        Addr tag;			// virtual page number tag
        Addr ppn;			// physical page number
        uint8_t xre;		// read permissions - VMEM_PERM_* mask
        uint8_t xwe;		// write permissions - VMEM_PERM_* mask
        uint8_t asn;		// address space number
        bool asma;			// address space match
        bool fonr;			// fault on read
        bool fonw;			// fault on write
        bool valid;			// valid page table entry

        Addr pageStart()
        {
            return ppn << PageShift;
        }

        void serialize(std::ostream &os);
        void unserialize(Checkpoint *cp, const std::string &section);
    };

};
#endif // __ARCH_ALPHA_PAGETABLE_H__

