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


#define m5_op 0x2
#define m5_op3 0x37

#include "m5ops.h"

#define INST(func, rs1, rs2, rd) \
        .long (m5_op) << 30 | (rd) << 25 | (m5_op3) << 19 | (func) << 7 |  \
              (rs1) << 14 | (rs2) << 0;


#define LEAF(func)                \
        .section ".text";         \
        .align   4;               \
        .global  func;            \
        .type    func, #function; \
func:

#define END(func)         \
        .size    func, (.-func)

#define DEBUGBREAK INST(debugbreak_func, 0, 0, 0)
#define M5EXIT INST(exit_func, 0, 0, 0)
#define PANIC INST(panic_func, 0, 0, 0)
#define READFILE INST(readfile_func, 0, 0, 0)

LEAF(m5_exit)
    retl
    M5EXIT
END(m5_exit)

LEAF(m5_panic)
    retl
    PANIC
END(m5_panic)

LEAF(m5_readfile)
    retl
    READFILE
END(m5_readfile)

LEAF(m5_debugbreak)
    retl
    DEBUGBREAK
END(m5_debugbreak)

/* !!!!!! All code below here just panics !!!!!! */
LEAF(arm)
    retl
    PANIC
END(arm)

LEAF(quiesce)
    retl
    PANIC
END(quiesce)

LEAF(quiesceNs)
    retl
    PANIC
END(quiesceNs)

LEAF(quiesceCycle)
    retl
    PANIC
END(quiesceCycle)

LEAF(quiesceTime)
    retl
    PANIC
END(quiesceTime)

LEAF(m5_initparam)
    retl
    PANIC
END(m5_initparam)

LEAF(m5_loadsymbol)
    retl
    PANIC
END(m5_loadsymbol)

LEAF(m5_reset_stats)
    retl
    PANIC
END(m5_reset_stats)

LEAF(m5_dump_stats)
    retl
    PANIC
END(m5_dump_stats)

LEAF(m5_dumpreset_stats)
    retl
    PANIC
END(m5_dumpreset_stats)

LEAF(m5_checkpoint)
    retl
    PANIC
END(m5_checkpoint)

LEAF(m5_switchcpu)
    retl
    PANIC
END(m5_switchcpu)

LEAF(m5_addsymbol)
    retl
    PANIC
END(m5_addsymbol)

LEAF(m5_anbegin)
    retl
    PANIC
END(m5_anbegin)

LEAF(m5_anwait)
    retl
    PANIC
END(m5_anwait)


