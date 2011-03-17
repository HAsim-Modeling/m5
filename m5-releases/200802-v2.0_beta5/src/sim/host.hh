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
 * Authors: Nathan L. Binkert
 */

/**
 * @file
 * Defines host-dependent types:
 * Counter, Tick, and (indirectly) {int,uint}{8,16,32,64}_t.
 */

#ifndef __HOST_HH__
#define __HOST_HH__

#include <inttypes.h>
#include <limits>


/** uint64_t constant */
#define ULL(N)		((uint64_t)N##ULL)
/** int64_t constant */
#define LL(N)		((int64_t)N##LL)

/** Statistics counter type.  Not much excuse for not using a 64-bit
 * integer here, but if you're desperate and only run short
 * simulations you could make this 32 bits.
 */
typedef int64_t Counter;

/**
 * Clock cycle count type.
 * @note using an unsigned breaks the cache.
 */
typedef int64_t Tick;

const Tick MaxTick = std::numeric_limits<Tick>::max();

/**
 * Address type
 * This will probably be moved somewhere else in the near future.
 * This should be at least as big as the biggest address width in use
 * in the system, which will probably be 64 bits.
 */
typedef uint64_t Addr;

const Addr MaxAddr = (Addr)-1;

#endif // __HOST_H__
