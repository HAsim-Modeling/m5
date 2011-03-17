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
 * Authors: Kevin T. Lim
 *          Gabe M. Black
 */

#ifndef __CPU_O3_REGFILE_HH__
#define __CPU_O3_REGFILE_HH__

#include "arch/isa_traits.hh"
#include "arch/types.hh"
#include "base/trace.hh"
#include "config/full_system.hh"
#include "cpu/o3/comm.hh"

#if FULL_SYSTEM
#include "arch/kernel_stats.hh"
#endif

#include <vector>

/**
 * Simple physical register file class.
 * Right now this is specific to Alpha until we decide if/how to make things
 * generic enough to support other ISAs.
 */
template <class Impl>
class PhysRegFile
{
  protected:
    typedef TheISA::IntReg IntReg;
    typedef TheISA::FloatReg FloatReg;
    typedef TheISA::FloatRegBits FloatRegBits;
    typedef TheISA::MiscRegFile MiscRegFile;
    typedef TheISA::MiscReg MiscReg;

    typedef union {
        FloatReg d;
        FloatRegBits q;
    } PhysFloatReg;

    // Note that most of the definitions of the IntReg, FloatReg, etc. exist
    // within the Impl/ISA class and not within this PhysRegFile class.

    // Will make these registers public for now, but they probably should
    // be private eventually with some accessor functions.
  public:
    typedef typename Impl::O3CPU O3CPU;

    /**
     * Constructs a physical register file with the specified amount of
     * integer and floating point registers.
     */
    PhysRegFile(O3CPU *_cpu, unsigned _numPhysicalIntRegs,
                unsigned _numPhysicalFloatRegs);

    //Everything below should be pretty well identical to the normal
    //register file that exists within AlphaISA class.
    //The duplication is unfortunate but it's better than having
    //different ways to access certain registers.

    /** Reads an integer register. */
    uint64_t readIntReg(PhysRegIndex reg_idx)
    {
        assert(reg_idx < numPhysicalIntRegs);

        DPRINTF(IEW, "RegFile: Access to int register %i, has data "
                "%#x\n", int(reg_idx), intRegFile[reg_idx]);
        return intRegFile[reg_idx];
    }

    FloatReg readFloatReg(PhysRegIndex reg_idx, int width)
    {
        // Remove the base Float reg dependency.
        reg_idx = reg_idx - numPhysicalIntRegs;

        assert(reg_idx < numPhysicalFloatRegs + numPhysicalIntRegs);

        FloatReg floatReg = floatRegFile[reg_idx].d;

        DPRINTF(IEW, "RegFile: Access to %d byte float register %i, has "
                "data %#x\n", int(reg_idx), floatRegFile[reg_idx].q);

        return floatReg;
    }

    /** Reads a floating point register (double precision). */
    FloatReg readFloatReg(PhysRegIndex reg_idx)
    {
        // Remove the base Float reg dependency.
        reg_idx = reg_idx - numPhysicalIntRegs;

        assert(reg_idx < numPhysicalFloatRegs + numPhysicalIntRegs);

        FloatReg floatReg = floatRegFile[reg_idx].d;

        DPRINTF(IEW, "RegFile: Access to float register %i, has "
                "data %#x\n", int(reg_idx), floatRegFile[reg_idx].q);

        return floatReg;
    }

    /** Reads a floating point register as an integer. */
    FloatRegBits readFloatRegBits(PhysRegIndex reg_idx, int width)
    {
        // Remove the base Float reg dependency.
        reg_idx = reg_idx - numPhysicalIntRegs;

        assert(reg_idx < numPhysicalFloatRegs + numPhysicalIntRegs);

        FloatRegBits floatRegBits = floatRegFile[reg_idx].q;

        DPRINTF(IEW, "RegFile: Access to float register %i as int, "
                "has data %#x\n", int(reg_idx), (uint64_t)floatRegBits);

        return floatRegBits;
    }

    FloatRegBits readFloatRegBits(PhysRegIndex reg_idx)
    {
        // Remove the base Float reg dependency.
        reg_idx = reg_idx - numPhysicalIntRegs;

        assert(reg_idx < numPhysicalFloatRegs + numPhysicalIntRegs);

        FloatRegBits floatRegBits = floatRegFile[reg_idx].q;

        DPRINTF(IEW, "RegFile: Access to float register %i as int, "
                "has data %#x\n", int(reg_idx), (uint64_t)floatRegBits);

        return floatRegBits;
    }

    /** Sets an integer register to the given value. */
    void setIntReg(PhysRegIndex reg_idx, uint64_t val)
    {
        assert(reg_idx < numPhysicalIntRegs);

        DPRINTF(IEW, "RegFile: Setting int register %i to %#x\n",
                int(reg_idx), val);

        if (reg_idx != TheISA::ZeroReg)
            intRegFile[reg_idx] = val;
    }

    /** Sets a single precision floating point register to the given value. */
    void setFloatReg(PhysRegIndex reg_idx, FloatReg val, int width)
    {
        // Remove the base Float reg dependency.
        reg_idx = reg_idx - numPhysicalIntRegs;

        assert(reg_idx < numPhysicalFloatRegs);

        DPRINTF(IEW, "RegFile: Setting float register %i to %#x\n",
                int(reg_idx), (uint64_t)val);

#if THE_ISA == ALPHA_ISA
        if (reg_idx != TheISA::ZeroReg)
#endif
            floatRegFile[reg_idx].d = val;
    }

    /** Sets a double precision floating point register to the given value. */
    void setFloatReg(PhysRegIndex reg_idx, FloatReg val)
    {
        // Remove the base Float reg dependency.
        reg_idx = reg_idx - numPhysicalIntRegs;

        assert(reg_idx < numPhysicalFloatRegs);

        DPRINTF(IEW, "RegFile: Setting float register %i to %#x\n",
                int(reg_idx), (uint64_t)val);

#if THE_ISA == ALPHA_ISA
        if (reg_idx != TheISA::ZeroReg)
#endif
            floatRegFile[reg_idx].d = val;
    }

    /** Sets a floating point register to the given integer value. */
    void setFloatRegBits(PhysRegIndex reg_idx, FloatRegBits val, int width)
    {
        // Remove the base Float reg dependency.
        reg_idx = reg_idx - numPhysicalIntRegs;

        assert(reg_idx < numPhysicalFloatRegs);

        DPRINTF(IEW, "RegFile: Setting float register %i to %#x\n",
                int(reg_idx), (uint64_t)val);

        floatRegFile[reg_idx].q = val;
    }

    void setFloatRegBits(PhysRegIndex reg_idx, FloatRegBits val)
    {
        // Remove the base Float reg dependency.
        reg_idx = reg_idx - numPhysicalIntRegs;

        assert(reg_idx < numPhysicalFloatRegs);

        DPRINTF(IEW, "RegFile: Setting float register %i to %#x\n",
                int(reg_idx), (uint64_t)val);

        floatRegFile[reg_idx].q = val;
    }

    MiscReg readMiscRegNoEffect(int misc_reg, unsigned thread_id)
    {
        return miscRegs[thread_id].readRegNoEffect(misc_reg);
    }

    MiscReg readMiscReg(int misc_reg, unsigned thread_id)
    {
        return miscRegs[thread_id].readReg(misc_reg, cpu->tcBase(thread_id));
    }

    void setMiscRegNoEffect(int misc_reg,
            const MiscReg &val, unsigned thread_id)
    {
        miscRegs[thread_id].setRegNoEffect(misc_reg, val);
    }

    void setMiscReg(int misc_reg, const MiscReg &val,
                               unsigned thread_id)
    {
        miscRegs[thread_id].setReg(misc_reg, val,
                                                    cpu->tcBase(thread_id));
    }

  public:
    /** (signed) integer register file. */
    IntReg *intRegFile;

    /** Floating point register file. */
    PhysFloatReg *floatRegFile;

    /** Miscellaneous register file. */
    MiscRegFile miscRegs[Impl::MaxThreads];

#if FULL_SYSTEM
  private:
    int intrflag;			// interrupt flag
#endif

  private:
    /** CPU pointer. */
    O3CPU *cpu;

  public:
    /** Number of physical integer registers. */
    unsigned numPhysicalIntRegs;
    /** Number of physical floating point registers. */
    unsigned numPhysicalFloatRegs;
};

template <class Impl>
PhysRegFile<Impl>::PhysRegFile(O3CPU *_cpu, unsigned _numPhysicalIntRegs,
                               unsigned _numPhysicalFloatRegs)
    : cpu(_cpu), numPhysicalIntRegs(_numPhysicalIntRegs),
      numPhysicalFloatRegs(_numPhysicalFloatRegs)
{
    intRegFile = new IntReg[numPhysicalIntRegs];
    floatRegFile = new PhysFloatReg[numPhysicalFloatRegs];

    for (int i = 0; i < Impl::MaxThreads; ++i) {
        miscRegs[i].clear();
    }

    memset(intRegFile, 0, sizeof(IntReg) * numPhysicalIntRegs);
    memset(floatRegFile, 0, sizeof(PhysFloatReg) * numPhysicalFloatRegs);
}

#endif
