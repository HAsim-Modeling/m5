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
 * Authors: Gabe Black
 *          Ali G. Saidi
 */

#include "arch/sparc/asi.hh"

namespace SparcISA
{
    bool AsiIsBlock(ASI asi)
    {
        return
            (asi == ASI_BLK_AIUP) ||
            (asi == ASI_BLK_AIUS) ||
            (asi == ASI_BLK_AIUP_L) ||
            (asi == ASI_BLK_AIUS_L) ||
            (asi == ASI_BLK_P) ||
            (asi == ASI_BLK_S) ||
            (asi == ASI_BLK_PL) ||
            (asi == ASI_BLK_SL);
    }

    bool AsiIsPrimary(ASI asi)
    {
        return
            (asi == ASI_AIUP) ||
            (asi == ASI_BLK_AIUP) ||
            (asi == ASI_AIUP_L) ||
            (asi == ASI_BLK_AIUP_L) ||
            (asi == ASI_LDTX_AIUP) ||
            (asi == ASI_LDTX_AIUP_L) ||
            (asi == ASI_P) ||
            (asi == ASI_PNF) ||
            (asi == ASI_PL) ||
            (asi == ASI_PNFL) ||
            (asi == ASI_PST8_P) ||
            (asi == ASI_PST16_P) ||
            (asi == ASI_PST32_P) ||
            (asi == ASI_PST8_PL) ||
            (asi == ASI_PST16_PL) ||
            (asi == ASI_PST32_PL) ||
            (asi == ASI_FL8_P) ||
            (asi == ASI_FL16_P) ||
            (asi == ASI_FL8_PL) ||
            (asi == ASI_FL16_PL) ||
            (asi == ASI_LDTX_P) ||
            (asi == ASI_LDTX_PL) ||
            (asi == ASI_BLK_P) ||
            (asi == ASI_BLK_PL);
    }

    bool AsiIsSecondary(ASI asi)
    {
        return
            (asi == ASI_AIUS) ||
            (asi == ASI_BLK_AIUS) ||
            (asi == ASI_AIUS_L) ||
            (asi == ASI_BLK_AIUS_L) ||
            (asi == ASI_LDTX_AIUS) ||
            (asi == ASI_LDTX_AIUS_L) ||
            (asi == ASI_S) ||
            (asi == ASI_SNF) ||
            (asi == ASI_SL) ||
            (asi == ASI_SNFL) ||
            (asi == ASI_PST8_S) ||
            (asi == ASI_PST16_S) ||
            (asi == ASI_PST32_S) ||
            (asi == ASI_PST8_SL) ||
            (asi == ASI_PST16_SL) ||
            (asi == ASI_PST32_SL) ||
            (asi == ASI_FL8_S) ||
            (asi == ASI_FL16_S) ||
            (asi == ASI_FL8_SL) ||
            (asi == ASI_FL16_SL) ||
            (asi == ASI_LDTX_S) ||
            (asi == ASI_LDTX_SL) ||
            (asi == ASI_BLK_S) ||
            (asi == ASI_BLK_SL);
    }

    bool AsiIsNucleus(ASI asi)
    {
        return
            (asi == ASI_N) ||
            (asi == ASI_NL) ||
            (asi == ASI_LDTX_N) ||
            (asi == ASI_LDTX_NL);
    }

    bool AsiIsAsIfUser(ASI asi)
    {
        return
            (asi == ASI_AIUP) ||
            (asi == ASI_AIUS) ||
            (asi == ASI_BLK_AIUP) ||
            (asi == ASI_BLK_AIUS) ||
            (asi == ASI_AIUP_L) ||
            (asi == ASI_AIUS_L) ||
            (asi == ASI_BLK_AIUP_L) ||
            (asi == ASI_BLK_AIUS_L) ||
            (asi == ASI_LDTX_AIUP) ||
            (asi == ASI_LDTX_AIUS) ||
            (asi == ASI_LDTX_AIUP_L) ||
            (asi == ASI_LDTX_AIUS_L);
    }

    bool AsiIsIO(ASI asi)
    {
        return
            (asi == ASI_REAL_IO) ||
            (asi == ASI_REAL_IO_L);
    }

    bool AsiIsReal(ASI asi)
    {
        return
            (asi == ASI_REAL) ||
            (asi == ASI_REAL_IO) ||
            (asi == ASI_REAL_L) ||
            (asi == ASI_REAL_IO_L) ||
            (asi == ASI_LDTX_REAL) ||
            (asi == ASI_LDTX_REAL_L);
    }

    bool AsiIsLittle(ASI asi)
    {
        return
            (asi == ASI_NL) ||
            (asi == ASI_AIUP_L) ||
            (asi == ASI_AIUS_L) ||
            (asi == ASI_REAL_L) ||
            (asi == ASI_REAL_IO_L) ||
            (asi == ASI_BLK_AIUP_L) ||
            (asi == ASI_BLK_AIUS_L) ||
            (asi == ASI_LDTX_AIUP_L) ||
            (asi == ASI_LDTX_AIUS_L) ||
            (asi == ASI_LDTX_REAL_L) ||
            (asi == ASI_LDTX_NL) ||
            (asi == ASI_PL) ||
            (asi == ASI_SL) ||
            (asi == ASI_PNFL) ||
            (asi == ASI_SNFL) ||
            (asi == ASI_PST8_PL) ||
            (asi == ASI_PST8_SL) ||
            (asi == ASI_PST16_PL) ||
            (asi == ASI_PST16_SL) ||
            (asi == ASI_PST32_PL) ||
            (asi == ASI_PST32_SL) ||
            (asi == ASI_FL8_PL) ||
            (asi == ASI_FL8_SL) ||
            (asi == ASI_FL16_PL) ||
            (asi == ASI_FL16_SL) ||
            (asi == ASI_LDTX_PL) ||
            (asi == ASI_LDTX_SL) ||
            (asi == ASI_BLK_PL) ||
            (asi == ASI_BLK_SL) ||
            (asi == ASI_LTX_L);
    }

    bool AsiIsTwin(ASI asi)
    {
        return
            (asi >= ASI_LDTX_AIUP &&
            asi <= ASI_LDTX_N &&
            asi != ASI_QUEUE) ||
            (asi >= ASI_LDTX_AIUP_L &&
            asi <= ASI_LDTX_NL &&
            asi != 0x2D) ||
            asi == ASI_LDTX_P ||
            asi == ASI_LDTX_S ||
            asi == ASI_LDTX_PL ||
            asi == ASI_LDTX_SL;
    }

    bool AsiIsPartialStore(ASI asi)
    {
        return
            (asi == ASI_PST8_P) ||
            (asi == ASI_PST8_S) ||
            (asi == ASI_PST16_P) ||
            (asi == ASI_PST16_S) ||
            (asi == ASI_PST32_P) ||
            (asi == ASI_PST32_S) ||
            (asi == ASI_PST8_PL) ||
            (asi == ASI_PST8_SL) ||
            (asi == ASI_PST16_PL) ||
            (asi == ASI_PST16_SL) ||
            (asi == ASI_PST32_PL) ||
            (asi == ASI_PST32_SL);
    }

    bool AsiIsFloatingLoad(ASI asi)
    {
        return
            (asi == ASI_FL8_P) ||
            (asi == ASI_FL8_S) ||
            (asi == ASI_FL16_P) ||
            (asi == ASI_FL16_S) ||
            (asi == ASI_FL8_PL) ||
            (asi == ASI_FL8_SL) ||
            (asi == ASI_FL16_PL) ||
            (asi == ASI_FL16_SL);
    }

    bool AsiIsNoFault(ASI asi)
    {
        return
            (asi == ASI_PNF) ||
            (asi == ASI_SNF) ||
            (asi == ASI_PNFL) ||
            (asi == ASI_SNFL);
    }

    bool AsiIsScratchPad(ASI asi)
    {
        return
            (asi == ASI_SCRATCHPAD) ||
            (asi == ASI_HYP_SCRATCHPAD);
    }

    bool AsiIsCmt(ASI asi)
    {
        return
            (asi == ASI_CMT_PER_STRAND) ||
            (asi == ASI_CMT_SHARED);
    }

    bool AsiIsQueue(ASI asi)
    {
        return asi == ASI_QUEUE;
    }

    bool AsiIsInterrupt(ASI asi)
    {
        return asi == ASI_SWVR_INTR_RECEIVE  ||
               asi == ASI_SWVR_UDB_INTR_W  ||
               asi == ASI_SWVR_UDB_INTR_R ;
    }

    bool AsiIsMmu(ASI asi)
    {
        return  asi == ASI_MMU ||
                asi == ASI_LSU_CONTROL_REG  ||
               (asi >= ASI_DMMU_CTXT_ZERO_TSB_BASE_PS0 &&
                asi <= ASI_IMMU_CTXT_ZERO_CONFIG) ||
               (asi >= ASI_DMMU_CTXT_NONZERO_TSB_BASE_PS0 &&
                asi <= ASI_IMMU_CTXT_NONZERO_CONFIG) ||
               (asi >= ASI_IMMU &&
                asi <= ASI_IMMU_TSB_PS1_PTR_REG) ||
               (asi >= ASI_ITLB_DATA_IN_REG  &&
                asi <= ASI_TLB_INVALIDATE_ALL);
    }

    bool AsiIsUnPriv(ASI asi)
    {
        return asi >= 0x80;
    }

    bool AsiIsPriv(ASI asi)
    {
        return asi <= 0x2f;
    }


    bool AsiIsHPriv(ASI asi)
    {
        return asi >= 0x30 && asi <= 0x7f;
    }

    bool AsiIsReg(ASI asi)
    {
        return AsiIsMmu(asi) || AsiIsScratchPad(asi) ||
               AsiIsSparcError(asi) || AsiIsInterrupt(asi)
               || AsiIsCmt(asi);
    }

    bool AsiIsSparcError(ASI asi)
    {
        return asi == ASI_SPARC_ERROR_EN_REG ||
               asi == ASI_SPARC_ERROR_STATUS_REG;
    }

}
