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

#ifndef __CPU_O3_STORE_SET_HH__
#define __CPU_O3_STORE_SET_HH__

#include <list>
#include <map>
#include <utility>
#include <vector>

#include "cpu/inst_seq.hh"
#include "sim/host.hh"

struct ltseqnum {
    bool operator()(const InstSeqNum &lhs, const InstSeqNum &rhs) const
    {
        return lhs > rhs;
    }
};

/**
 * Implements a store set predictor for determining if memory
 * instructions are dependent upon each other.  See paper "Memory
 * Dependence Prediction using Store Sets" by Chrysos and Emer.  SSID
 * stands for Store Set ID, SSIT stands for Store Set ID Table, and
 * LFST is Last Fetched Store Table.
 */
class StoreSet
{
  public:
    typedef unsigned SSID;

  public:
    /** Default constructor.  init() must be called prior to use. */
    StoreSet() { };

    /** Creates store set predictor with given table sizes. */
    StoreSet(int SSIT_size, int LFST_size);

    /** Default destructor. */
    ~StoreSet();

    /** Initializes the store set predictor with the given table sizes. */
    void init(int SSIT_size, int LFST_size);

    /** Records a memory ordering violation between the younger load
     * and the older store. */
    void violation(Addr store_PC, Addr load_PC);

    /** Inserts a load into the store set predictor.  This does nothing but
     * is included in case other predictors require a similar function.
     */
    void insertLoad(Addr load_PC, InstSeqNum load_seq_num);

    /** Inserts a store into the store set predictor.  Updates the
     * LFST if the store has a valid SSID. */
    void insertStore(Addr store_PC, InstSeqNum store_seq_num,
                     unsigned tid);

    /** Checks if the instruction with the given PC is dependent upon
     * any store.  @return Returns the sequence number of the store
     * instruction this PC is dependent upon.  Returns 0 if none.
     */
    InstSeqNum checkInst(Addr PC);

    /** Records this PC/sequence number as issued. */
    void issued(Addr issued_PC, InstSeqNum issued_seq_num, bool is_store);

    /** Squashes for a specific thread until the given sequence number. */
    void squash(InstSeqNum squashed_num, unsigned tid);

    /** Resets all tables. */
    void clear();

    /** Debug function to dump the contents of the store list. */
    void dump();

  private:
    /** Calculates the index into the SSIT based on the PC. */
    inline int calcIndex(Addr PC)
    { return (PC >> offsetBits) & indexMask; }

    /** Calculates a Store Set ID based on the PC. */
    inline SSID calcSSID(Addr PC)
    { return ((PC ^ (PC >> 10)) % LFSTSize); }

    /** The Store Set ID Table. */
    std::vector<SSID> SSIT;

    /** Bit vector to tell if the SSIT has a valid entry. */
    std::vector<bool> validSSIT;

    /** Last Fetched Store Table. */
    std::vector<InstSeqNum> LFST;

    /** Bit vector to tell if the LFST has a valid entry. */
    std::vector<bool> validLFST;

    /** Map of stores that have been inserted into the store set, but
     * not yet issued or squashed.
     */
    std::map<InstSeqNum, int, ltseqnum> storeList;

    typedef std::map<InstSeqNum, int, ltseqnum>::iterator SeqNumMapIt;

    /** Store Set ID Table size, in entries. */
    int SSITSize;

    /** Last Fetched Store Table size, in entries. */
    int LFSTSize;

    /** Mask to obtain the index. */
    int indexMask;

    // HACK: Hardcoded for now.
    int offsetBits;
};

#endif // __CPU_O3_STORE_SET_HH__
