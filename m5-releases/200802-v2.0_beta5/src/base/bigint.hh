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
 */

#include "base/misc.hh"

#include <iostream>

#ifndef __BASE_BIGINT_HH__
#define __BASE_BIGINT_HH__
// Create a couple of large int types for atomic reads
struct m5_twin64_t {
    uint64_t a;
    uint64_t b;
    m5_twin64_t()
    {}
    m5_twin64_t(const uint64_t x)
    {
        a = x;
        b = x;
    }
    inline m5_twin64_t& operator=(const uint64_t x)
    {
        a = x;
        b = x;
        return *this;
    }

    operator uint64_t()
    {
        panic("Tried to cram a twin64_t into an integer!\n");
        return a;
    }
};

struct m5_twin32_t {
    uint32_t a;
    uint32_t b;
    m5_twin32_t()
    {}
    m5_twin32_t(const uint32_t x)
    {
        a = x;
        b = x;
    }
    inline m5_twin32_t& operator=(const uint32_t x)
    {
        a = x;
        b = x;
        return *this;
    }

    operator uint32_t()
    {
        panic("Tried to cram a twin32_t into an integer!\n");
        return a;
    }
};


// This is for twin loads (two 64 bit values), not 1 128 bit value (as far as
// endian conversion is concerned!
typedef m5_twin64_t Twin64_t;
typedef m5_twin32_t Twin32_t;

// Output operator overloads
std::ostream & operator << (std::ostream & os, const Twin64_t & t);
std::ostream & operator << (std::ostream & os, const Twin32_t & t);

#endif // __BASE_BIGINT_HH__

