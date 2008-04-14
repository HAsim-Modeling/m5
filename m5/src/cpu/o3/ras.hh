/*
 * Copyright (c) 2004, 2005
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

#ifndef __CPU_O3_RAS_HH__
#define __CPU_O3_RAS_HH__

#include "sim/host.hh"
#include <vector>

/** Return address stack class, implements a simple RAS. */
class ReturnAddrStack
{
  public:
    /** Creates a return address stack, but init() must be called prior to
     *  use.
     */
    ReturnAddrStack() {}

    /** Initializes RAS with a specified number of entries.
     *  @param numEntries Number of entries in the RAS.
     */
    void init(unsigned numEntries);

    void reset();

    /** Returns the top address on the RAS. */
    Addr top()
    { return addrStack[tos]; }

    /** Returns the index of the top of the RAS. */
    unsigned topIdx()
    { return tos; }

    /** Pushes an address onto the RAS. */
    void push(const Addr &return_addr);

    /** Pops the top address from the RAS. */
    void pop();

    /** Changes index to the top of the RAS, and replaces the top address with
     *  a new target.
     *  @param top_entry_idx The index of the RAS that will now be the top.
     *  @param restored_target The new target address of the new top of the RAS.
     */
    void restore(unsigned top_entry_idx, const Addr &restored_target);

  private:
    /** Increments the top of stack index. */
    inline void incrTos()
    { if (++tos == numEntries) tos = 0; }

    /** Decrements the top of stack index. */
    inline void decrTos()
    { tos = (tos == 0 ? numEntries - 1 : tos - 1); }

    /** The RAS itself. */
    std::vector<Addr> addrStack;

    /** The number of entries in the RAS. */
    unsigned numEntries;

    /** The number of used entries in the RAS. */
    unsigned usedEntries;

    /** The top of stack index. */
    unsigned tos;
};

#endif // __CPU_O3_RAS_HH__
