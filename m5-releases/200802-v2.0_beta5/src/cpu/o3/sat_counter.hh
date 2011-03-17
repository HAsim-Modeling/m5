/*
 * Copyright (c) 2005, 2006
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
 * Authors: Kevin T. Lim
 */

#ifndef __CPU_O3_SAT_COUNTER_HH__
#define __CPU_O3_SAT_COUNTER_HH__

#include "base/misc.hh"
#include "sim/host.hh"

/**
 * Private counter class for the internal saturating counters.
 * Implements an n bit saturating counter and provides methods to
 * increment, decrement, and read it.
 * @todo Consider making this something that more closely mimics a
 * built in class so you can use ++ or --.
 */
class SatCounter
{
  public:
    /**
     * Constructor for the counter.
     */
    SatCounter()
        : initialVal(0), counter(0)
    { }

    /**
     * Constructor for the counter.
     * @param bits How many bits the counter will have.
     */
    SatCounter(unsigned bits)
        : initialVal(0), maxVal((1 << bits) - 1), counter(0)
    { }

    /**
     * Constructor for the counter.
     * @param bits How many bits the counter will have.
     * @param initial_val Starting value for each counter.
     */
    SatCounter(unsigned bits, uint8_t initial_val)
        : initialVal(initialVal), maxVal((1 << bits) - 1), counter(initial_val)
    {
        // Check to make sure initial value doesn't exceed the max
        // counter value.
        if (initial_val > maxVal) {
            fatal("BP: Initial counter value exceeds max size.");
        }
    }

    /**
     * Sets the number of bits.
     */
    void setBits(unsigned bits) { maxVal = (1 << bits) - 1; }

    void reset() { counter = initialVal; }

    /**
     * Increments the counter's current value.
     */
    void increment()
    {
        if (counter < maxVal) {
            ++counter;
        }
    }

    /**
     * Decrements the counter's current value.
     */
    void decrement()
    {
        if (counter > 0) {
            --counter;
        }
    }

    /**
     * Read the counter's value.
     */
    const uint8_t read() const
    { return counter; }

  private:
    uint8_t initialVal;
    uint8_t maxVal;
    uint8_t counter;
};

#endif // __CPU_O3_SAT_COUNTER_HH__
