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
 * Authors: Ali G. Saidi
 *          Nathan L. Binkert
 */

#include "arch/isa_traits.hh"
#include "base/bigint.hh"
#include "mem/packet.hh"
#include "sim/byteswap.hh"

#ifndef __MEM_PACKET_ACCESS_HH__
#define __MEM_PACKET_ACCESS_HH__
// The memory system needs to have an endianness. This is the easiest
// way to deal with it for now. At some point, we will have to remove
// these functions and make the users do their own byte swapping since
// the memory system does not in fact have an endianness.

/** return the value of what is pointed to in the packet. */
template <typename T>
inline T
Packet::get()
{
    assert(staticData || dynamicData);
    assert(sizeof(T) <= size);
    return TheISA::gtoh(*(T*)data);
}

/** set the value in the data pointer to v. */
template <typename T>
inline void
Packet::set(T v)
{
    assert(sizeof(T) <= size);
    *(T*)data = TheISA::htog(v);
}

#endif //__MEM_PACKET_ACCESS_HH__
