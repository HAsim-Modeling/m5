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
 * Authors: Ali G. Saidi
 */

/** @file
 * Malta PCI interface CSRs
 */

#ifndef __MALTA_PCHIP_HH__
#define __MALTA_PCHIP_HH__

#include "dev/mips/malta.hh"
#include "base/range.hh"
#include "dev/io_device.hh"
#include "params/MaltaPChip.hh"

/**
 * A very simple implementation of the Malta PCI interface chips.
 */
class MaltaPChip : public BasicPioDevice
{
  protected:

    static const Addr MaltaPciBus0Config = ULL(0x801fe000000);

    /** Pchip control register */
    uint64_t pctl;

    /** Window Base addresses */
    uint64_t wsba[4];

    /** Window masks */
    uint64_t wsm[4];

    /** Translated Base Addresses */
    uint64_t tba[4];

  public:
    typedef MaltaPChipParams Params;

    const Params *
    params() const
    {
        return dynamic_cast<const Params *>(_params);
    }
  public:
    /**
     * Register the PChip with the mmu and init all wsba, wsm, and tba to 0
     * @param p pointer to the parameters struct
     */
    MaltaPChip(const Params *p);

    /**
     * Translate a PCI bus address to a memory address for DMA.
     * @todo Andrew says this needs to be fixed. What's wrong with it?
     * @param busAddr PCI address to translate.
     * @return memory system address
     */
    Addr translatePciToDma(Addr busAddr);

    Addr calcConfigAddr(int bus, int dev, int func);

    virtual Tick read(PacketPtr pkt);
    virtual Tick write(PacketPtr pkt);

    /**
     * Serialize this object to the given output stream.
     * @param os The stream to serialize to.
     */
    virtual void serialize(std::ostream &os);

    /**
     * Reconstruct the state of this object from a checkpoint.
     * @param cp The checkpoint use.
     * @param section The section name of this object
     */
    virtual void unserialize(Checkpoint *cp, const std::string &section);
};

#endif // __TSUNAMI_PCHIP_HH__
