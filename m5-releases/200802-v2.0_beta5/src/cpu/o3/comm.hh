/*
 * Copyright (c) 2004, 2005, 2006
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

#ifndef __CPU_O3_COMM_HH__
#define __CPU_O3_COMM_HH__

#include <vector>

#include "sim/faults.hh"
#include "cpu/inst_seq.hh"
#include "sim/host.hh"

// Typedef for physical register index type. Although the Impl would be the
// most likely location for this, there are a few classes that need this
// typedef yet are not templated on the Impl. For now it will be defined here.
typedef short int PhysRegIndex;

/** Struct that defines the information passed from fetch to decode. */
template<class Impl>
struct DefaultFetchDefaultDecode {
    typedef typename Impl::DynInstPtr DynInstPtr;

    int size;

    DynInstPtr insts[Impl::MaxWidth];
    Fault fetchFault;
    InstSeqNum fetchFaultSN;
    bool clearFetchFault;
};

/** Struct that defines the information passed from decode to rename. */
template<class Impl>
struct DefaultDecodeDefaultRename {
    typedef typename Impl::DynInstPtr DynInstPtr;

    int size;

    DynInstPtr insts[Impl::MaxWidth];
};

/** Struct that defines the information passed from rename to IEW. */
template<class Impl>
struct DefaultRenameDefaultIEW {
    typedef typename Impl::DynInstPtr DynInstPtr;

    int size;

    DynInstPtr insts[Impl::MaxWidth];
};

/** Struct that defines the information passed from IEW to commit. */
template<class Impl>
struct DefaultIEWDefaultCommit {
    typedef typename Impl::DynInstPtr DynInstPtr;

    int size;

    DynInstPtr insts[Impl::MaxWidth];

    bool squash[Impl::MaxThreads];
    bool branchMispredict[Impl::MaxThreads];
    bool branchTaken[Impl::MaxThreads];
    Addr mispredPC[Impl::MaxThreads];
    Addr nextPC[Impl::MaxThreads];
    Addr nextNPC[Impl::MaxThreads];
    Addr nextMicroPC[Impl::MaxThreads];
    InstSeqNum squashedSeqNum[Impl::MaxThreads];

    bool includeSquashInst[Impl::MaxThreads];
};

template<class Impl>
struct IssueStruct {
    typedef typename Impl::DynInstPtr DynInstPtr;

    int size;

    DynInstPtr insts[Impl::MaxWidth];
};

/** Struct that defines all backwards communication. */
template<class Impl>
struct TimeBufStruct {
    struct decodeComm {
        bool squash;
        bool predIncorrect;
        uint64_t branchAddr;

        InstSeqNum doneSeqNum;

        // @todo: Might want to package this kind of branch stuff into a single
        // struct as it is used pretty frequently.
        bool branchMispredict;
        bool branchTaken;
        Addr mispredPC;
        Addr nextPC;
        Addr nextNPC;
        Addr nextMicroPC;

        unsigned branchCount;
    };

    decodeComm decodeInfo[Impl::MaxThreads];

    struct renameComm {
    };

    renameComm renameInfo[Impl::MaxThreads];

    struct iewComm {
        // Also eventually include skid buffer space.
        bool usedIQ;
        unsigned freeIQEntries;
        bool usedLSQ;
        unsigned freeLSQEntries;

        unsigned iqCount;
        unsigned ldstqCount;

        unsigned dispatched;
        unsigned dispatchedToLSQ;
    };

    iewComm iewInfo[Impl::MaxThreads];

    struct commitComm {
        bool usedROB;
        unsigned freeROBEntries;
        bool emptyROB;

        bool squash;
        bool robSquashing;

        bool branchMispredict;
        bool branchTaken;
        Addr mispredPC;
        Addr nextPC;
        Addr nextNPC;
        Addr nextMicroPC;

        // Represents the instruction that has either been retired or
        // squashed.  Similar to having a single bus that broadcasts the
        // retired or squashed sequence number.
        InstSeqNum doneSeqNum;

        //Just in case we want to do a commit/squash on a cycle
        //(necessary for multiple ROBs?)
        bool commitInsts;
        InstSeqNum squashSeqNum;

        // Communication specifically to the IQ to tell the IQ that it can
        // schedule a non-speculative instruction.
        InstSeqNum nonSpecSeqNum;

        // Hack for now to send back an uncached access to the IEW stage.
        typedef typename Impl::DynInstPtr DynInstPtr;
        bool uncached;
        DynInstPtr uncachedLoad;

        bool interruptPending;
        bool clearInterrupt;
    };

    commitComm commitInfo[Impl::MaxThreads];

    bool decodeBlock[Impl::MaxThreads];
    bool decodeUnblock[Impl::MaxThreads];
    bool renameBlock[Impl::MaxThreads];
    bool renameUnblock[Impl::MaxThreads];
    bool iewBlock[Impl::MaxThreads];
    bool iewUnblock[Impl::MaxThreads];
    bool commitBlock[Impl::MaxThreads];
    bool commitUnblock[Impl::MaxThreads];
};

#endif //__CPU_O3_COMM_HH__
