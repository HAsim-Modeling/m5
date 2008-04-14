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

#ifndef __CPU_O3_BTB_HH__
#define __CPU_O3_BTB_HH__

#include "base/misc.hh"
#include "sim/host.hh"

class DefaultBTB
{
  private:
    struct BTBEntry
    {
        BTBEntry()
            : tag(0), target(0), valid(false)
        {
        }

        /** The entry's tag. */
        Addr tag;

        /** The entry's target. */
        Addr target;

        /** The entry's thread id. */
        unsigned tid;

        /** Whether or not the entry is valid. */
        bool valid;
    };

  public:
    /** Creates a BTB with the given number of entries, number of bits per
     *  tag, and instruction offset amount.
     *  @param numEntries Number of entries for the BTB.
     *  @param tagBits Number of bits for each tag in the BTB.
     *  @param instShiftAmt Offset amount for instructions to ignore alignment.
     */
    DefaultBTB(unsigned numEntries, unsigned tagBits,
               unsigned instShiftAmt);

    void reset();

    /** Looks up an address in the BTB. Must call valid() first on the address.
     *  @param inst_PC The address of the branch to look up.
     *  @param tid The thread id.
     *  @return Returns the target of the branch.
     */
    Addr lookup(const Addr &inst_PC, unsigned tid);

    /** Checks if a branch is in the BTB.
     *  @param inst_PC The address of the branch to look up.
     *  @param tid The thread id.
     *  @return Whether or not the branch exists in the BTB.
     */
    bool valid(const Addr &inst_PC, unsigned tid);

    /** Updates the BTB with the target of a branch.
     *  @param inst_PC The address of the branch being updated.
     *  @param target_PC The target address of the branch.
     *  @param tid The thread id.
     */
    void update(const Addr &inst_PC, const Addr &target_PC,
                unsigned tid);

  private:
    /** Returns the index into the BTB, based on the branch's PC.
     *  @param inst_PC The branch to look up.
     *  @return Returns the index into the BTB.
     */
    inline unsigned getIndex(const Addr &inst_PC);

    /** Returns the tag bits of a given address.
     *  @param inst_PC The branch's address.
     *  @return Returns the tag bits.
     */
    inline Addr getTag(const Addr &inst_PC);

    /** The actual BTB. */
    std::vector<BTBEntry> btb;

    /** The number of entries in the BTB. */
    unsigned numEntries;

    /** The index mask. */
    unsigned idxMask;

    /** The number of tag bits per entry. */
    unsigned tagBits;

    /** The tag mask. */
    unsigned tagMask;

    /** Number of bits to shift PC when calculating index. */
    unsigned instShiftAmt;

    /** Number of bits to shift PC when calculating tag. */
    unsigned tagShiftAmt;
};

#endif // __CPU_O3_BTB_HH__
