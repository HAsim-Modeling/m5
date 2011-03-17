/* $Id$ */

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

#ifndef __M5OP_H__
#define __M5OP_H__

#include <stdint.h>

void arm(uint64_t address);
void quiesce(void);
void quiesceNs(uint64_t ns);
void quiesceCycle(uint64_t cycles);
uint64_t quiesceTime(void);

void m5_exit(uint64_t ns_delay);
uint64_t m5_initparam(void);
void m5_checkpoint(uint64_t ns_delay, uint64_t ns_period);
void m5_reset_stats(uint64_t ns_delay, uint64_t ns_period);
void m5_dump_stats(uint64_t ns_delay, uint64_t ns_period);
void m5_dumpreset_stats(uint64_t ns_delay, uint64_t ns_period);
uint64_t m5_readfile(void *buffer, uint64_t len, uint64_t offset);
void m5_debugbreak(void);
void m5_switchcpu(void);
void m5_addsymbol(uint64_t addr, char *symbol);
void m5_panic(void);
void m5_anbegin(uint64_t s);
void m5_anwait(uint64_t s, uint64_t w);

#endif // __M5OP_H__
