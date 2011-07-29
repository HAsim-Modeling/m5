/*
 * Copyright (c) 2003, 2004, 2005
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
 * Authors: Steven K. Reinhardt
 *          Gabe M. Black
 */

#ifndef __ARCH_ALPHA_IPR_HH__
#define __ARCH_ALPHA_IPR_HH__

namespace AlphaISA
{
    ////////////////////////////////////////////////////////////////////////
    //
    //  Internal Processor Reigsters
    //
    enum md_ipr_names
    {
        RAW_IPR_ISR = 0x100,		// interrupt summary register
        RAW_IPR_ITB_TAG = 0x101,	// ITLB tag register
        RAW_IPR_ITB_PTE = 0x102,	// ITLB page table entry register
        RAW_IPR_ITB_ASN = 0x103,	// ITLB address space register
        RAW_IPR_ITB_PTE_TEMP = 0x104,	// ITLB page table entry temp register
        RAW_IPR_ITB_IA = 0x105,		// ITLB invalidate all register
        RAW_IPR_ITB_IAP = 0x106,	// ITLB invalidate all process register
        RAW_IPR_ITB_IS = 0x107,		// ITLB invalidate select register
        RAW_IPR_SIRR = 0x108,		// software interrupt request register
        RAW_IPR_ASTRR = 0x109,		// asynchronous system trap request register
        RAW_IPR_ASTER = 0x10a,		// asynchronous system trap enable register
        RAW_IPR_EXC_ADDR = 0x10b,	// exception address register
        RAW_IPR_EXC_SUM = 0x10c,	// exception summary register
        RAW_IPR_EXC_MASK = 0x10d,	// exception mask register
        RAW_IPR_PAL_BASE = 0x10e,	// PAL base address register
        RAW_IPR_ICM = 0x10f,		// instruction current mode
        RAW_IPR_IPLR = 0x110,		// interrupt priority level register
        RAW_IPR_INTID = 0x111,		// interrupt ID register
        RAW_IPR_IFAULT_VA_FORM = 0x112,	// formatted faulting virtual addr register
        RAW_IPR_IVPTBR = 0x113,		// virtual page table base register
        RAW_IPR_HWINT_CLR = 0x115,	// H/W interrupt clear register
        RAW_IPR_SL_XMIT = 0x116,	// serial line transmit register
        RAW_IPR_SL_RCV = 0x117,		// serial line receive register
        RAW_IPR_ICSR = 0x118,		// instruction control and status register
        RAW_IPR_IC_FLUSH = 0x119,	// instruction cache flush control
        RAW_IPR_IC_PERR_STAT = 0x11a,	// inst cache parity error status register
        RAW_IPR_PMCTR = 0x11c,		// performance counter register

        // PAL temporary registers...
        // register meanings gleaned from osfpal.s source code
        RAW_IPR_PALtemp0 = 0x140,	// local scratch
        RAW_IPR_PALtemp1 = 0x141,	// local scratch
        RAW_IPR_PALtemp2 = 0x142,	// entUna
        RAW_IPR_PALtemp3 = 0x143,	// CPU specific impure area pointer
        RAW_IPR_PALtemp4 = 0x144,	// memory management temp
        RAW_IPR_PALtemp5 = 0x145,	// memory management temp
        RAW_IPR_PALtemp6 = 0x146,	// memory management temp
        RAW_IPR_PALtemp7 = 0x147,	// entIF
        RAW_IPR_PALtemp8 = 0x148,	// intmask
        RAW_IPR_PALtemp9 = 0x149,	// entSys
        RAW_IPR_PALtemp10 = 0x14a,	// ??
        RAW_IPR_PALtemp11 = 0x14b,	// entInt
        RAW_IPR_PALtemp12 = 0x14c,	// entArith
        RAW_IPR_PALtemp13 = 0x14d,	// reserved for platform specific PAL
        RAW_IPR_PALtemp14 = 0x14e,	// reserved for platform specific PAL
        RAW_IPR_PALtemp15 = 0x14f,	// reserved for platform specific PAL
        RAW_IPR_PALtemp16 = 0x150,	// scratch / whami<7:0> / mces<4:0>
        RAW_IPR_PALtemp17 = 0x151,	// sysval
        RAW_IPR_PALtemp18 = 0x152,	// usp
        RAW_IPR_PALtemp19 = 0x153,	// ksp
        RAW_IPR_PALtemp20 = 0x154,	// PTBR
        RAW_IPR_PALtemp21 = 0x155,	// entMM
        RAW_IPR_PALtemp22 = 0x156,	// kgp
        RAW_IPR_PALtemp23 = 0x157,	// PCBB

        RAW_IPR_DTB_ASN = 0x200,	// DTLB address space number register
        RAW_IPR_DTB_CM = 0x201,		// DTLB current mode register
        RAW_IPR_DTB_TAG = 0x202,	// DTLB tag register
        RAW_IPR_DTB_PTE = 0x203,	// DTLB page table entry register
        RAW_IPR_DTB_PTE_TEMP = 0x204,	// DTLB page table entry temporary register

        RAW_IPR_MM_STAT = 0x205,	// data MMU fault status register
        RAW_IPR_VA = 0x206,		// fault virtual address register
        RAW_IPR_VA_FORM = 0x207,	// formatted virtual address register
        RAW_IPR_MVPTBR = 0x208,		// MTU virtual page table base register
        RAW_IPR_DTB_IAP = 0x209,	// DTLB invalidate all process register
        RAW_IPR_DTB_IA = 0x20a,		// DTLB invalidate all register
        RAW_IPR_DTB_IS = 0x20b,		// DTLB invalidate single register
        RAW_IPR_ALT_MODE = 0x20c,	// alternate mode register
        RAW_IPR_CC = 0x20d,		// cycle counter register
        RAW_IPR_CC_CTL = 0x20e,		// cycle counter control register
        RAW_IPR_MCSR = 0x20f,		// MTU control register

        RAW_IPR_DC_FLUSH = 0x210,
        RAW_IPR_DC_PERR_STAT = 0x212,	// Dcache parity error status register
        RAW_IPR_DC_TEST_CTL = 0x213,	// Dcache test tag control register
        RAW_IPR_DC_TEST_TAG = 0x214,	// Dcache test tag register
        RAW_IPR_DC_TEST_TAG_TEMP = 0x215, // Dcache test tag temporary register
        RAW_IPR_DC_MODE = 0x216,	// Dcache mode register
        RAW_IPR_MAF_MODE = 0x217,	// miss address file mode register

        MaxInternalProcRegs		// number of IPR registers
    };

    enum MiscRegIpr
    {
        //Write only
        MinWriteOnlyIpr,
        IPR_HWINT_CLR = MinWriteOnlyIpr,
        IPR_SL_XMIT,
        IPR_DC_FLUSH,
        IPR_IC_FLUSH,
        IPR_ALT_MODE,
        IPR_DTB_IA,
        IPR_DTB_IAP,
        IPR_ITB_IA,
        MaxWriteOnlyIpr,
        IPR_ITB_IAP = MaxWriteOnlyIpr,

        //Read only
        MinReadOnlyIpr,
        IPR_INTID = MinReadOnlyIpr,
        IPR_SL_RCV,
        IPR_MM_STAT,
        IPR_ITB_PTE_TEMP,
        MaxReadOnlyIpr,
        IPR_DTB_PTE_TEMP = MaxReadOnlyIpr,

        IPR_ISR,
        IPR_ITB_TAG,
        IPR_ITB_PTE,
        IPR_ITB_ASN,
        IPR_ITB_IS,
        IPR_SIRR,
        IPR_ASTRR,
        IPR_ASTER,
        IPR_EXC_ADDR,
        IPR_EXC_SUM,
        IPR_EXC_MASK,
        IPR_PAL_BASE,
        IPR_ICM,
        IPR_IPLR,
        IPR_IFAULT_VA_FORM,
        IPR_IVPTBR,
        IPR_ICSR,
        IPR_IC_PERR_STAT,
        IPR_PMCTR,

        // PAL temporary registers...
        // register meanings gleaned from osfpal.s source code
        IPR_PALtemp0,
        IPR_PALtemp1,
        IPR_PALtemp2,
        IPR_PALtemp3,
        IPR_PALtemp4,
        IPR_PALtemp5,
        IPR_PALtemp6,
        IPR_PALtemp7,
        IPR_PALtemp8,
        IPR_PALtemp9,
        IPR_PALtemp10,
        IPR_PALtemp11,
        IPR_PALtemp12,
        IPR_PALtemp13,
        IPR_PALtemp14,
        IPR_PALtemp15,
        IPR_PALtemp16,
        IPR_PALtemp17,
        IPR_PALtemp18,
        IPR_PALtemp19,
        IPR_PALtemp20,
        IPR_PALtemp21,
        IPR_PALtemp22,
        IPR_PALtemp23,

        IPR_DTB_ASN,
        IPR_DTB_CM,
        IPR_DTB_TAG,
        IPR_DTB_PTE,

        IPR_VA,
        IPR_VA_FORM,
        IPR_MVPTBR,
        IPR_DTB_IS,
        IPR_CC,
        IPR_CC_CTL,
        IPR_MCSR,

        IPR_DC_PERR_STAT,
        IPR_DC_TEST_CTL,
        IPR_DC_TEST_TAG,
        IPR_DC_TEST_TAG_TEMP,
        IPR_DC_MODE,
        IPR_MAF_MODE,

        NumInternalProcRegs		// number of IPR registers
    };

    inline bool IprIsWritable(int index)
    {
        return index < MinReadOnlyIpr || index > MaxReadOnlyIpr;
    }

    inline bool IprIsReadable(int index)
    {
        return index < MinWriteOnlyIpr || index > MaxWriteOnlyIpr;
    }

    extern md_ipr_names MiscRegIndexToIpr[NumInternalProcRegs];
    extern int IprToMiscRegIndex[MaxInternalProcRegs];

    void initializeIprTable();
}

#endif