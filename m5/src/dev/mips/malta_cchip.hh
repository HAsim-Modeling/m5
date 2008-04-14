/*
 * Copyright (c) 2007 MIPS Technologies, Inc.  All Rights Reserved
 *
 * This software is part of the M5 simulator.
 *
 * THIS IS A LEGAL AGREEMENT.  BY DOWNLOADING, USING, COPYING, CREATING
 * DERIVATIVE WORKS, AND/OR DISTRIBUTING THIS SOFTWARE YOU ARE AGREEING
 * TO THESE TERMS AND CONDITIONS.
 *
 * Permission is granted to use, copy, create derivative works and
 * distribute this software and such derivative works for any purpose,
 * so long as (1) the copyright notice above, this grant of permission,
 * and the disclaimer below appear in all copies and derivative works
 * made, (2) the copyright notice above is augmented as appropriate to
 * reflect the addition of any new copyrightable work in a derivative
 * work (e.g., Copyright (c) <Publication Year> Copyright Owner), and (3)
 * the name of MIPS Technologies, Inc. (¡ÈMIPS¡É) is not used in any
 * advertising or publicity pertaining to the use or distribution of
 * this software without specific, written prior authorization.
 *
 * THIS SOFTWARE IS PROVIDED ¡ÈAS IS.¡É  MIPS MAKES NO WARRANTIES AND
 * DISCLAIMS ALL WARRANTIES, WHETHER EXPRESS, STATUTORY, IMPLIED OR
 * OTHERWISE, INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND
 * NON-INFRINGEMENT OF THIRD PARTY RIGHTS, REGARDING THIS SOFTWARE.
 * IN NO EVENT SHALL MIPS BE LIABLE FOR ANY DAMAGES, INCLUDING DIRECT,
 * INDIRECT, INCIDENTAL, CONSEQUENTIAL, SPECIAL, OR PUNITIVE DAMAGES OF
 * ANY KIND OR NATURE, ARISING OUT OF OR IN CONNECTION WITH THIS AGREEMENT,
 * THIS SOFTWARE AND/OR THE USE OF THIS SOFTWARE, WHETHER SUCH LIABILITY
 * IS ASSERTED ON THE BASIS OF CONTRACT, TORT (INCLUDING NEGLIGENCE OR
 * STRICT LIABILITY), OR OTHERWISE, EVEN IF MIPS HAS BEEN WARNED OF THE
 * POSSIBILITY OF ANY SUCH LOSS OR DAMAGE IN ADVANCE.
 *
 * Authors: Richard Strong 
 *
 */

/** @file
 * Emulation of the Malta CChip CSRs
 */

#ifndef __MALTA_CCHIP_HH__
#define __MALTA_CCHIP_HH__

#include "dev/mips/malta.hh"
#include "base/range.hh"
#include "dev/io_device.hh"
#include "params/MaltaCChip.hh"

/**
 * Malta CChip CSR Emulation. This device includes all the interrupt
 * handling code for the chipset.
 */
class MaltaCChip : public BasicPioDevice
{
  protected:
    /**
     * pointer to the malta object.
     * This is our access to all the other malta
     * devices.
     */
    Malta *malta;

    /**
     * The dims are device interrupt mask registers.
     * One exists for each CPU, the DRIR X DIM = DIR
     */
    //uint64_t dim[Malta::Max_CPUs];

    /**
     * The dirs are device interrupt registers.
     * One exists for each CPU, the DRIR X DIM = DIR
     */
    //uint64_t dir[Malta::Max_CPUs];

    /**
     * This register contains bits for each PCI interrupt
     * that can occur.
     */
    //uint64_t drir;

    /** Indicator of which CPUs have an IPI interrupt */
    //uint64_t ipint;

    /** Indicator of which CPUs have an RTC interrupt */
    //uint64_t itint;

  public:
    typedef MaltaCChipParams Params;

    const Params *
    params() const
    {
        return dynamic_cast<const Params *>(_params);
    }

    /**
     * Initialize the Malta CChip by setting all of the
     * device register to 0.
     * @param p params struct
     */
    MaltaCChip(Params *p);

    virtual Tick read(PacketPtr pkt);

    virtual Tick write(PacketPtr pkt);

    /**
     * post an RTC interrupt to the CPU
     */
    void postRTC();

    /**
     * post an interrupt to the CPU.
     * @param interrupt the interrupt number to post (0-7)
     */
    void postIntr(uint32_t interrupt);

    /**
     * clear an interrupt previously posted to the CPU.
     * @param interrupt the interrupt number to post (0-7)
     */
    void clearIntr(uint32_t interrupt);

    /**
     * post an ipi interrupt  to the CPU.
     * @param ipintr the cpu number to clear(bitvector)
     */
    void clearIPI(uint64_t ipintr);

    /**
     * clear a timer interrupt previously posted to the CPU.
     * @param itintr the cpu number to clear(bitvector)
     */
    void clearITI(uint64_t itintr);

    /**
     * request an interrupt be posted to the CPU.
     * @param ipreq the cpu number to interrupt(bitvector)
     */
    void reqIPI(uint64_t ipreq);


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

#endif // __MALTA_CCHIP_HH__
