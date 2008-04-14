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
 *          Ali G. Saidi
 */

#include "base/fenv.hh"
#include "base/intmath.hh"
#include "base/misc.hh"
#include "base/random.hh"
#include "sim/serialize.hh"

using namespace std;

Random::Random()
{
    // default random seed taken from original source
    init(5489);
}

Random::Random(uint32_t s)
{
    init(s);
}

Random::Random(uint32_t init_key[], int key_length)
{
    init(init_key, key_length);
}

Random::~Random()
{
}

// To preserve the uniform random distribution between min and max,
// and allow all numbers to be represented, we generate a uniform
// random number to the nearest power of two greater than max.  If
// this number doesn't fall between 0 and max, we try again.  Anything
// else would skew the distribution.
uint32_t
Random::genrand(uint32_t max)
{
    int log = ceilLog2(max);
    int shift = (sizeof(uint32_t) * 8 - log);
    uint32_t random;

    do {
        random = genrand() >> shift;
    } while (random > max);

    return random;
}

uint64_t
Random::genrand(uint64_t max)
{
    int log = ceilLog2(max);
    int shift = (sizeof(uint64_t) * 8 - log);
    uint64_t random;

    do {
        random = (uint64_t)genrand() << 32 | (uint64_t)genrand();
        random = random >> shift;
    } while (random > max);

    return random;
}

void
Random::serialize(const string &base, ostream &os)
{
    int length = N;
    paramOut(os, base + ".mti", mti);
    paramOut(os, base + ".length", length);
    arrayParamOut(os, base + ".data", mt, length);
}

void
Random::unserialize(const string &base, Checkpoint *cp, const string &section)
{
    int length;

    paramIn(cp, section, base + ".mti", mti);
    paramIn(cp, section, base + ".length", length);
    if (length != N)
        panic("cant unserialize random number data. length != %d\n", length);

    arrayParamIn(cp, section, base + ".data", mt, length);
}

Random random_mt;
