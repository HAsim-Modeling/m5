/*
 * Copyright (c) 2004
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
 *          Ali G. Saidi
 *          Nathan L. Binkert
 */

//The purpose of this file is to provide endainness conversion utility
//functions. Depending on the endianness of the guest system, either
//the LittleEndianGuest or BigEndianGuest namespace is used.

#ifndef __SIM_BYTE_SWAP_HH__
#define __SIM_BYTE_SWAP_HH__

#include "base/bigint.hh"
#include "base/misc.hh"
#include "sim/host.hh"

// This lets us figure out what the byte order of the host system is
#if defined(linux)
#include <endian.h>
// If this is a linux system, lets used the optimized definitions if they exist.
// If one doesn't exist, we pretty much get what is listed below, so it all
// works out
#include <byteswap.h>
#elif defined (__sun)
#include <sys/isa_defs.h>
#else
#include <machine/endian.h>
#endif

#if defined(__APPLE__)
#include <libkern/OSByteOrder.h>
#endif

enum ByteOrder {BigEndianByteOrder, LittleEndianByteOrder};

//These functions actually perform the swapping for parameters
//of various bit lengths
static inline uint64_t
swap_byte64(uint64_t x)
{
#if defined(linux)
    return bswap_64(x);
#elif defined(__APPLE__)
    return OSSwapInt64(x);
#else
    return  (uint64_t)((((uint64_t)(x) & 0xff) << 56) |
            ((uint64_t)(x) & 0xff00ULL) << 40 |
            ((uint64_t)(x) & 0xff0000ULL) << 24 |
            ((uint64_t)(x) & 0xff000000ULL) << 8 |
            ((uint64_t)(x) & 0xff00000000ULL) >> 8 |
            ((uint64_t)(x) & 0xff0000000000ULL) >> 24 |
            ((uint64_t)(x) & 0xff000000000000ULL) >> 40 |
            ((uint64_t)(x) & 0xff00000000000000ULL) >> 56) ;
#endif
}

static inline uint32_t
swap_byte32(uint32_t x)
{
#if defined(linux)
    return bswap_32(x);
#elif defined(__APPLE__)
    return OSSwapInt32(x);
#else
    return  (uint32_t)(((uint32_t)(x) & 0xff) << 24 |
            ((uint32_t)(x) & 0xff00) << 8 | ((uint32_t)(x) & 0xff0000) >> 8 |
            ((uint32_t)(x) & 0xff000000) >> 24);
#endif
}

static inline uint16_t
swap_byte16(uint16_t x)
{
#if defined(linux)
    return bswap_16(x);
#elif defined(__APPLE__)
    return OSSwapInt16(x);
#else
    return (uint16_t)(((uint16_t)(x) & 0xff) << 8 |
                      ((uint16_t)(x) & 0xff00) >> 8);
#endif
}

// This function lets the compiler figure out how to call the
// swap_byte functions above for different data types.  Since the
// sizeof() values are known at compile time, it should inline to a
// direct call to the right swap_byteNN() function.
template <typename T>
static inline T swap_byte(T x) {
    if (sizeof(T) == 8)
        return swap_byte64((uint64_t)x);
    else if (sizeof(T) == 4)
        return swap_byte32((uint32_t)x);
    else if (sizeof(T) == 2)
        return swap_byte16((uint16_t)x);
    else if (sizeof(T) == 1)
        return x;
    else
        panic("Can't byte-swap values larger than 64 bits");
}

template<>
static inline Twin64_t swap_byte<Twin64_t>(Twin64_t x)
{
    x.a = swap_byte(x.a);
    x.b = swap_byte(x.b);
    return x;
}

template<>
static inline Twin32_t swap_byte<Twin32_t>(Twin32_t x)
{
    x.a = swap_byte(x.a);
    x.b = swap_byte(x.b);
    return x;
}

//The conversion functions with fixed endianness on both ends don't need to
//be in a namespace
template <typename T> static inline T betole(T value) {return swap_byte(value);}
template <typename T> static inline T letobe(T value) {return swap_byte(value);}

//For conversions not involving the guest system, we can define the functions
//conditionally based on the BYTE_ORDER macro and outside of the namespaces
#if defined(_BIG_ENDIAN) || !defined(_LITTLE_ENDIAN) && BYTE_ORDER == BIG_ENDIAN
const ByteOrder HostByteOrder = BigEndianByteOrder;
template <typename T> static inline T htole(T value) {return swap_byte(value);}
template <typename T> static inline T letoh(T value) {return swap_byte(value);}
template <typename T> static inline T htobe(T value) {return value;}
template <typename T> static inline T betoh(T value) {return value;}
#elif defined(_LITTLE_ENDIAN) || BYTE_ORDER == LITTLE_ENDIAN
const ByteOrder HostByteOrder = LittleEndianByteOrder;
template <typename T> static inline T htole(T value) {return value;}
template <typename T> static inline T letoh(T value) {return value;}
template <typename T> static inline T htobe(T value) {return swap_byte(value);}
template <typename T> static inline T betoh(T value) {return swap_byte(value);}
#else
        #error Invalid Endianess
#endif

namespace BigEndianGuest
{
    const bool ByteOrderDiffers = (HostByteOrder != BigEndianByteOrder);
    template <typename T>
    static inline T gtole(T value) {return betole(value);}
    template <typename T>
    static inline T letog(T value) {return letobe(value);}
    template <typename T>
    static inline T gtobe(T value) {return value;}
    template <typename T>
    static inline T betog(T value) {return value;}
    template <typename T>
    static inline T htog(T value) {return htobe(value);}
    template <typename T>
    static inline T gtoh(T value) {return betoh(value);}
}

namespace LittleEndianGuest
{
    const bool ByteOrderDiffers = (HostByteOrder != LittleEndianByteOrder);
    template <typename T>
    static inline T gtole(T value) {return value;}
    template <typename T>
    static inline T letog(T value) {return value;}
    template <typename T>
    static inline T gtobe(T value) {return letobe(value);}
    template <typename T>
    static inline T betog(T value) {return betole(value);}
    template <typename T>
    static inline T htog(T value) {return htole(value);}
    template <typename T>
    static inline T gtoh(T value) {return letoh(value);}
}
#endif // __SIM_BYTE_SWAP_HH__
