/*
 * Copyright (c) 2002, 2003, 2004, 2005
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

#ifndef __MIPS_ACCESS_H__
#define __MIPS_ACCESS_H__

/** @file
 * System Console Memory Mapped Register Definition
 */

#define MIPS_ACCESS_VERSION (1305)
#define CONSOLE_START_ADDRESS 0xBFD00F00
#define REG_OFFSET 1
#define UART8250_BASE 0xBFD003F8
#define UART8250_END 7*REG_OFFSET
#ifdef CONSOLE
typedef unsigned uint32_t;
typedef unsigned long uint64_t;
#endif

// This structure hacked up from simos
struct MipsAccess
{
                uint32_t	inputChar;		// 00: Placeholder for input
    uint32_t	last_offset;		// 04: must be first field
    uint32_t	version;		// 08:
    uint32_t	numCPUs;		// 0C:
    uint32_t	intrClockFrequency;	// 10: Hz

    // Loaded kernel
    uint32_t	kernStart;		// 14:
    uint32_t	kernEnd;		// 18:
    uint32_t	entryPoint;		// 1c:

                // console simple output stuff
                uint32_t	outputChar;		// 20: Placeholder for output

    // console disk stuff
    uint32_t	diskUnit;		// 24:
    uint32_t	diskCount;		// 28:
    uint32_t	diskPAddr;		// 2c:
    uint32_t	diskBlock;		// 30:
    uint32_t	diskOperation;		// 34:

                // MP boot
    uint32_t	cpuStack[64];	        // 70:

    /* XXX There appears to be a problem in accessing
     * unit64_t in the console.c file. They are treated
     * like uint32_int and result in the wrong address for
     * everything below. This problem should be investigated.
     */
    uint64_t	cpuClock;		// 38: MHz
    uint64_t	mem_size;		// 40:
};

#endif // __MIPS_ACCESS_H__
