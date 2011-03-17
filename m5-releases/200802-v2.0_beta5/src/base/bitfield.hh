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
 * Authors: Steven K. Reinhardt
 *          Nathan L. Binkert
 */

#ifndef __BASE_BITFIELD_HH__
#define __BASE_BITFIELD_HH__

#include <inttypes.h>

/**
 * Generate a 64-bit mask of 'nbits' 1s, right justified.
 */
inline uint64_t
mask(int nbits)
{
    return (nbits == 64) ? (uint64_t)-1LL : (1ULL << nbits) - 1;
}



/**
 * Extract the bitfield from position 'first' to 'last' (inclusive)
 * from 'val' and right justify it.  MSB is numbered 63, LSB is 0.
 */
template <class T>
inline
T
bits(T val, int first, int last)
{
    int nbits = first - last + 1;
    return (val >> last) & mask(nbits);
}

/**
 * Extract the bit from this position from 'val' and right justify it.
 */
template <class T>
inline
T
bits(T val, int bit)
{
    return bits(val, bit, bit);
}

/**
 * Mask off the given bits in place like bits() but without shifting.
 * msb = 63, lsb = 0
 */
template <class T>
inline
T
mbits(T val, int first, int last)
{
    return val & (mask(first+1) & ~mask(last));
}

inline uint64_t
mask(int first, int last)
{
    return mbits((uint64_t)-1LL, first, last);
}

/**
 * Sign-extend an N-bit value to 64 bits.
 */
template <int N>
inline
int64_t
sext(uint64_t val)
{
    int sign_bit = bits(val, N-1, N-1);
    return sign_bit ? (val | ~mask(N)) : val;
}

/**
 * Return val with bits first to last set to bit_val
 */
template <class T, class B>
inline
T
insertBits(T val, int first, int last, B bit_val)
{
    T t_bit_val = bit_val;
    T bmask = mask(first - last + 1) << last;
    return ((t_bit_val << last) & bmask) | (val & ~bmask);
}

/**
 * Overloaded for access to only one bit in value
 */
template <class T, class B>
inline
T
insertBits(T val, int bit, B bit_val)
{
    return insertBits(val, bit, bit, bit_val);
}

/**
 * A convenience function to replace bits first to last of val with bit_val
 * in place.
 */
template <class T, class B>
inline
void
replaceBits(T& val, int first, int last, B bit_val)
{
    val = insertBits(val, first, last, bit_val);
}

/** Overloaded function to allow to access only 1 bit*/
template <class T, class B>
inline
void
replaceBits(T& val, int bit, B bit_val)
{
    val = insertBits(val, bit, bit, bit_val);
}
/**
 * Returns the bit position of the MSB that is set in the input
 */
inline
int
findMsbSet(uint64_t val) {
    int msb = 0;
    if (!val)
        return 0;
    if (bits(val, 63,32)) { msb += 32; val >>= 32; }
    if (bits(val, 31,16)) { msb += 16; val >>= 16; }
    if (bits(val, 15,8))  { msb += 8;  val >>= 8;  }
    if (bits(val, 7,4))   { msb += 4;  val >>= 4;  }
    if (bits(val, 3,2))   { msb += 2;  val >>= 2;  }
    if (bits(val, 1,1))   { msb += 1; }
    return msb;
}

#endif // __BASE_BITFIELD_HH__
