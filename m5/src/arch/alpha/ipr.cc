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
 * Authors: Gabe M. Black
 */

#include <assert.h>
#include <string.h>

#include "arch/alpha/ipr.hh"

namespace AlphaISA
{
    md_ipr_names MiscRegIndexToIpr[NumInternalProcRegs] =
    {
        //Write only
        RAW_IPR_HWINT_CLR,	// H/W interrupt clear register
        RAW_IPR_SL_XMIT,	// serial line transmit register
        RAW_IPR_DC_FLUSH,
        RAW_IPR_IC_FLUSH,	// instruction cache flush control
        RAW_IPR_ALT_MODE,	// alternate mode register
        RAW_IPR_DTB_IA,		// DTLB invalidate all register
        RAW_IPR_DTB_IAP,	// DTLB invalidate all process register
        RAW_IPR_ITB_IA,		// ITLB invalidate all register
        RAW_IPR_ITB_IAP,	// ITLB invalidate all process register

        //Read only
        RAW_IPR_INTID,		// interrupt ID register
        RAW_IPR_SL_RCV,		// serial line receive register
        RAW_IPR_MM_STAT,	// data MMU fault status register
        RAW_IPR_ITB_PTE_TEMP,	// ITLB page table entry temp register
        RAW_IPR_DTB_PTE_TEMP,	// DTLB page table entry temporary register

        RAW_IPR_ISR,		// interrupt summary register
        RAW_IPR_ITB_TAG,	// ITLB tag register
        RAW_IPR_ITB_PTE,	// ITLB page table entry register
        RAW_IPR_ITB_ASN,	// ITLB address space register
        RAW_IPR_ITB_IS,		// ITLB invalidate select register
        RAW_IPR_SIRR,		// software interrupt request register
        RAW_IPR_ASTRR,		// asynchronous system trap request register
        RAW_IPR_ASTER,		// asynchronous system trap enable register
        RAW_IPR_EXC_ADDR,	// exception address register
        RAW_IPR_EXC_SUM,	// exception summary register
        RAW_IPR_EXC_MASK,	// exception mask register
        RAW_IPR_PAL_BASE,	// PAL base address register
        RAW_IPR_ICM,		// instruction current mode
        RAW_IPR_IPLR,		// interrupt priority level register
        RAW_IPR_IFAULT_VA_FORM,	// formatted faulting virtual addr register
        RAW_IPR_IVPTBR,		// virtual page table base register
        RAW_IPR_ICSR,		// instruction control and status register
        RAW_IPR_IC_PERR_STAT,	// inst cache parity error status register
        RAW_IPR_PMCTR,		// performance counter register

        // PAL temporary registers...
        // register meanings gleaned from osfpal.s source code
        RAW_IPR_PALtemp0,	// local scratch
        RAW_IPR_PALtemp1,	// local scratch
        RAW_IPR_PALtemp2,	// entUna
        RAW_IPR_PALtemp3,	// CPU specific impure area pointer
        RAW_IPR_PALtemp4,	// memory management temp
        RAW_IPR_PALtemp5,	// memory management temp
        RAW_IPR_PALtemp6,	// memory management temp
        RAW_IPR_PALtemp7,	// entIF
        RAW_IPR_PALtemp8,	// intmask
        RAW_IPR_PALtemp9,	// entSys
        RAW_IPR_PALtemp10,	// ??
        RAW_IPR_PALtemp11,	// entInt
        RAW_IPR_PALtemp12,	// entArith
        RAW_IPR_PALtemp13,	// reserved for platform specific PAL
        RAW_IPR_PALtemp14,	// reserved for platform specific PAL
        RAW_IPR_PALtemp15,	// reserved for platform specific PAL
        RAW_IPR_PALtemp16,	// scratch / whami<7:0> / mces<4:0>
        RAW_IPR_PALtemp17,	// sysval
        RAW_IPR_PALtemp18,	// usp
        RAW_IPR_PALtemp19,	// ksp
        RAW_IPR_PALtemp20,	// PTBR
        RAW_IPR_PALtemp21,	// entMM
        RAW_IPR_PALtemp22,	// kgp
        RAW_IPR_PALtemp23,	// PCBB

        RAW_IPR_DTB_ASN,	// DTLB address space number register
        RAW_IPR_DTB_CM,		// DTLB current mode register
        RAW_IPR_DTB_TAG,	// DTLB tag register
        RAW_IPR_DTB_PTE,	// DTLB page table entry register

        RAW_IPR_VA,		// fault virtual address register
        RAW_IPR_VA_FORM,	// formatted virtual address register
        RAW_IPR_MVPTBR,		// MTU virtual page table base register
        RAW_IPR_DTB_IS,		// DTLB invalidate single register
        RAW_IPR_CC,		// cycle counter register
        RAW_IPR_CC_CTL,		// cycle counter control register
        RAW_IPR_MCSR,		// MTU control register

        RAW_IPR_DC_PERR_STAT,	// Dcache parity error status register
        RAW_IPR_DC_TEST_CTL,	// Dcache test tag control register
        RAW_IPR_DC_TEST_TAG,	// Dcache test tag register
        RAW_IPR_DC_TEST_TAG_TEMP, // Dcache test tag temporary register
        RAW_IPR_DC_MODE,	// Dcache mode register
        RAW_IPR_MAF_MODE	// miss address file mode register
    };

    int IprToMiscRegIndex[MaxInternalProcRegs];

    void initializeIprTable()
    {
        static bool initialized = false;
        if(initialized)
            return;

        memset(IprToMiscRegIndex, -1, MaxInternalProcRegs * sizeof(int));

        for(int x = 0; x < NumInternalProcRegs; x++)
            IprToMiscRegIndex[MiscRegIndexToIpr[x]] = x;
    }
}

