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


#define arm_func 0x00
#define quiesce_func 0x01
#define quiescens_func 0x02
#define quiescecycle_func 0x03
#define quiescetime_func 0x04
#define ivlb 0x10 // obsolete
#define ivle 0x11 // obsolete
#define exit_old_func 0x20 // deprecated!
#define exit_func 0x21
#define initparam_func 0x30
#define loadsymbol_func 0x31
#define resetstats_func 0x40
#define dumpstats_func 0x41
#define dumprststats_func 0x42
#define ckpt_func 0x43
#define readfile_func 0x50
#define debugbreak_func 0x51
#define switchcpu_func 0x52
#define addsymbol_func 0x53
#define panic_func     0x54
#define anbegin_func     0x55
#define anwait_func     0x56

