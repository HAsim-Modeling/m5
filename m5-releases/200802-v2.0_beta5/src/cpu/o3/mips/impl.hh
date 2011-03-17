/*
 * Copyright (c) 2006
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
 *          Korey L. Sewell
 */

#ifndef __CPU_O3_MIPS_IMPL_HH__
#define __CPU_O3_MIPS_IMPL_HH__

#include "arch/mips/isa_traits.hh"

#include "cpu/o3/mips/params.hh"
#include "cpu/o3/cpu_policy.hh"


// Forward declarations.
template <class Impl>
class MipsDynInst;

template <class Impl>
class MipsO3CPU;

/** Implementation specific struct that defines several key types to the
 *  CPU, the stages within the CPU, the time buffers, and the DynInst.
 *  The struct defines the ISA, the CPU policy, the specific DynInst, the
 *  specific O3CPU, and all of the structs from the time buffers to do
 *  communication.
 *  This is one of the key things that must be defined for each hardware
 *  specific CPU implementation.
 */
struct MipsSimpleImpl
{
    /** The type of MachInst. */
    typedef TheISA::MachInst MachInst;

    /** The CPU policy to be used, which defines all of the CPU stages. */
    typedef SimpleCPUPolicy<MipsSimpleImpl> CPUPol;

    /** The DynInst type to be used. */
    typedef MipsDynInst<MipsSimpleImpl> DynInst;

    /** The refcounted DynInst pointer to be used.  In most cases this is
     *  what should be used, and not DynInst *.
     */
    typedef RefCountingPtr<DynInst> DynInstPtr;

    /** The O3CPU type to be used. */
    typedef MipsO3CPU<MipsSimpleImpl> O3CPU;

    /** Same typedef, but for CPUType.  BaseDynInst may not always use
     * an O3 CPU, so it's clearer to call it CPUType instead in that
     * case.
     */
    typedef O3CPU CPUType;

    /** The Params to be passed to each stage. */
    typedef MipsSimpleParams Params;

    enum {
      MaxWidth = 8,
      MaxThreads = 4
    };
};

/** The O3Impl to be used. */
typedef MipsSimpleImpl O3CPUImpl;

#endif // __CPU_O3_MIPS_IMPL_HH__
