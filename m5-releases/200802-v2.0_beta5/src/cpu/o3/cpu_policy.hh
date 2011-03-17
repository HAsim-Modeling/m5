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

#ifndef __CPU_O3_CPU_POLICY_HH__
#define __CPU_O3_CPU_POLICY_HH__

#include "cpu/o3/bpred_unit.hh"
#include "cpu/o3/free_list.hh"
#include "cpu/o3/inst_queue.hh"
#include "cpu/o3/lsq.hh"
#include "cpu/o3/lsq_unit.hh"
#include "cpu/o3/mem_dep_unit.hh"
#include "cpu/o3/regfile.hh"
#include "cpu/o3/rename_map.hh"
#include "cpu/o3/rob.hh"
#include "cpu/o3/store_set.hh"

#include "cpu/o3/commit.hh"
#include "cpu/o3/decode.hh"
#include "cpu/o3/fetch.hh"
#include "cpu/o3/iew.hh"
#include "cpu/o3/rename.hh"

#include "cpu/o3/comm.hh"

/**
 * Struct that defines the key classes to be used by the CPU.  All
 * classes use the typedefs defined here to determine what are the
 * classes of the other stages and communication buffers.  In order to
 * change a structure such as the IQ, simply change the typedef here
 * to use the desired class instead, and recompile.  In order to
 * create a different CPU to be used simultaneously with this one, see
 * the alpha_impl.hh file for instructions.
 */
template<class Impl>
struct SimpleCPUPolicy
{
    /** Typedef for the branch prediction unit (which includes the BP,
     * RAS, and BTB).
     */
    typedef BPredUnit<Impl> BPredUnit;
    /** Typedef for the register file.  Most classes assume a unified
     * physical register file.
     */
    typedef PhysRegFile<Impl> RegFile;
    /** Typedef for the freelist of registers. */
    typedef SimpleFreeList FreeList;
    /** Typedef for the rename map. */
    typedef SimpleRenameMap RenameMap;
    /** Typedef for the ROB. */
    typedef ROB<Impl> ROB;
    /** Typedef for the instruction queue/scheduler. */
    typedef InstructionQueue<Impl> IQ;
    /** Typedef for the memory dependence unit. */
    typedef MemDepUnit<StoreSet, Impl> MemDepUnit;
    /** Typedef for the LSQ. */
    typedef LSQ<Impl> LSQ;
    /** Typedef for the thread-specific LSQ units. */
    typedef LSQUnit<Impl> LSQUnit;

    /** Typedef for fetch. */
    typedef DefaultFetch<Impl> Fetch;
    /** Typedef for decode. */
    typedef DefaultDecode<Impl> Decode;
    /** Typedef for rename. */
    typedef DefaultRename<Impl> Rename;
    /** Typedef for Issue/Execute/Writeback. */
    typedef DefaultIEW<Impl> IEW;
    /** Typedef for commit. */
    typedef DefaultCommit<Impl> Commit;

    /** The struct for communication between fetch and decode. */
    typedef DefaultFetchDefaultDecode<Impl> FetchStruct;

    /** The struct for communication between decode and rename. */
    typedef DefaultDecodeDefaultRename<Impl> DecodeStruct;

    /** The struct for communication between rename and IEW. */
    typedef DefaultRenameDefaultIEW<Impl> RenameStruct;

    /** The struct for communication between IEW and commit. */
    typedef DefaultIEWDefaultCommit<Impl> IEWStruct;

    /** The struct for communication within the IEW stage. */
    typedef IssueStruct<Impl> IssueStruct;

    /** The struct for all backwards communication. */
    typedef TimeBufStruct<Impl> TimeStruct;

};

#endif //__CPU_O3_CPU_POLICY_HH__
