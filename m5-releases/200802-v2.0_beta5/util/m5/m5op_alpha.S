/*
 * Copyright (c) 2003 - 2006
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
 *          Ali G. Saidi
 */


#define m5_op 0x01

#include "m5ops.h"

#define INST(op, ra, rb, func) \
        .long (((op) << 26) | ((ra) << 21) | ((rb) << 16) | (func))

#define LEAF(func)    \
        .align 3;     \
        .globl  func; \
        .ent    func; \
func:

#define RET           \
        ret     ($26)

#define END(func)     \
        .end func

#define	ARM(reg) INST(m5_op, reg, 0, arm_func)
#define QUIESCE INST(m5_op, 0, 0, quiesce_func)
#define QUIESCENS(r1) INST(m5_op, r1, 0, quiescens_func)
#define QUIESCECYC(r1) INST(m5_op, r1, 0, quiescecycle_func)
#define QUIESCETIME INST(m5_op, 0, 0, quiescetime_func)
#define M5EXIT(reg) INST(m5_op, reg, 0, exit_func)
#define INITPARAM(reg) INST(m5_op, reg, 0, initparam_func)
#define LOADSYMBOL(reg) INST(m5_op, reg, 0, loadsymbol_func)
#define RESET_STATS(r1, r2) INST(m5_op, r1, r2, resetstats_func)
#define DUMP_STATS(r1, r2) INST(m5_op, r1, r2, dumpstats_func)
#define DUMPRST_STATS(r1, r2) INST(m5_op, r1, r2, dumprststats_func)
#define CHECKPOINT(r1, r2) INST(m5_op, r1, r2, ckpt_func)
#define READFILE INST(m5_op, 0, 0, readfile_func)
#define DEBUGBREAK INST(m5_op, 0, 0, debugbreak_func)
#define SWITCHCPU INST(m5_op, 0, 0, switchcpu_func)
#define ADDSYMBOL(r1,r2) INST(m5_op, r1, r2, addsymbol_func)
#define PANIC INST(m5_op, 0, 0, panic_func)
#define AN_BEGIN(r1) INST(m5_op, r1, 0, anbegin_func)
#define AN_WAIT(r1,r2) INST(m5_op, r1, r2, anwait_func)

        .set noreorder

        .align 4
LEAF(arm)
        ARM(16)
        RET
END(arm)

        .align 4
LEAF(quiesce)
        QUIESCE
        RET
END(quiesce)

        .align 4
LEAF(quiesceNs)
        QUIESCENS(16)
        RET
END(quiesceNs)

        .align 4
LEAF(quiesceCycle)
        QUIESCECYC(16)
        RET
END(quiesceCycle)

        .align 4
LEAF(quiesceTime)
        QUIESCETIME
        RET
END(quiesceTime)

        .align 4
LEAF(m5_exit)
        M5EXIT(16)
        RET
END(m5_exit)

        .align 4
LEAF(m5_initparam)
        INITPARAM(0)
        RET
END(m5_initparam)

        .align 4
LEAF(m5_loadsymbol)
        LOADSYMBOL(0)
        RET
END(m5_loadsymbol)

        .align 4
LEAF(m5_reset_stats)
        RESET_STATS(16, 17)
        RET
END(m5_reset_stats)

        .align 4
LEAF(m5_dump_stats)
        DUMP_STATS(16, 17)
        RET
END(m5_dump_stats)

        .align 4
LEAF(m5_dumpreset_stats)
        DUMPRST_STATS(16, 17)
        RET
END(m5_dumpreset_stats)

        .align 4
LEAF(m5_checkpoint)
        CHECKPOINT(16, 17)
        RET
END(m5_checkpoint)

        .align 4
LEAF(m5_readfile)
        READFILE
        RET
END(m5_readfile)

        .align 4
LEAF(m5_debugbreak)
        DEBUGBREAK
        RET
END(m5_debugbreak)

        .align 4
LEAF(m5_switchcpu)
        SWITCHCPU
        RET
END(m5_switchcpu)

        .align 4
LEAF(m5_addsymbol)
        ADDSYMBOL(16, 17)
        RET
END(m5_addsymbol)

        .align 4
LEAF(m5_panic)
        PANIC
        RET
END(m5_panic)


        .align 4
LEAF(m5_anbegin)
        AN_BEGIN(16)
        RET
END(m5_anbegin)


        .align 4
LEAF(m5_anwait)
        AN_WAIT(16,17)
        RET
END(m5_anwait)


