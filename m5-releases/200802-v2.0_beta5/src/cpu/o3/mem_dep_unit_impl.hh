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

#include <map>

#include "cpu/o3/inst_queue.hh"
#include "cpu/o3/mem_dep_unit.hh"

template <class MemDepPred, class Impl>
MemDepUnit<MemDepPred, Impl>::MemDepUnit()
    : loadBarrier(false), loadBarrierSN(0), storeBarrier(false),
      storeBarrierSN(0), iqPtr(NULL)
{
}

template <class MemDepPred, class Impl>
MemDepUnit<MemDepPred, Impl>::MemDepUnit(Params *params)
    : depPred(params->SSITSize, params->LFSTSize), loadBarrier(false),
      loadBarrierSN(0), storeBarrier(false), storeBarrierSN(0), iqPtr(NULL)
{
    DPRINTF(MemDepUnit, "Creating MemDepUnit object.\n");
}

template <class MemDepPred, class Impl>
MemDepUnit<MemDepPred, Impl>::~MemDepUnit()
{
    for (int tid=0; tid < Impl::MaxThreads; tid++) {

        ListIt inst_list_it = instList[tid].begin();

        MemDepHashIt hash_it;

        while (!instList[tid].empty()) {
            hash_it = memDepHash.find((*inst_list_it)->seqNum);

            assert(hash_it != memDepHash.end());

            memDepHash.erase(hash_it);

            instList[tid].erase(inst_list_it++);
        }
    }

#ifdef DEBUG
    assert(MemDepEntry::memdep_count == 0);
#endif
}

template <class MemDepPred, class Impl>
std::string
MemDepUnit<MemDepPred, Impl>::name() const
{
    return "memdepunit";
}

template <class MemDepPred, class Impl>
void
MemDepUnit<MemDepPred, Impl>::init(Params *params, int tid)
{
    DPRINTF(MemDepUnit, "Creating MemDepUnit %i object.\n",tid);

    id = tid;

    depPred.init(params->SSITSize, params->LFSTSize);
}

template <class MemDepPred, class Impl>
void
MemDepUnit<MemDepPred, Impl>::regStats()
{
    insertedLoads
        .name(name() + ".memDep.insertedLoads")
        .desc("Number of loads inserted to the mem dependence unit.");

    insertedStores
        .name(name() + ".memDep.insertedStores")
        .desc("Number of stores inserted to the mem dependence unit.");

    conflictingLoads
        .name(name() + ".memDep.conflictingLoads")
        .desc("Number of conflicting loads.");

    conflictingStores
        .name(name() + ".memDep.conflictingStores")
        .desc("Number of conflicting stores.");
}

template <class MemDepPred, class Impl>
void
MemDepUnit<MemDepPred, Impl>::switchOut()
{
    assert(instList[0].empty());
    assert(instsToReplay.empty());
    assert(memDepHash.empty());
    // Clear any state.
    for (int i = 0; i < Impl::MaxThreads; ++i) {
        instList[i].clear();
    }
    instsToReplay.clear();
    memDepHash.clear();
}

template <class MemDepPred, class Impl>
void
MemDepUnit<MemDepPred, Impl>::takeOverFrom()
{
    // Be sure to reset all state.
    loadBarrier = storeBarrier = false;
    loadBarrierSN = storeBarrierSN = 0;
    depPred.clear();
}

template <class MemDepPred, class Impl>
void
MemDepUnit<MemDepPred, Impl>::setIQ(InstructionQueue<Impl> *iq_ptr)
{
    iqPtr = iq_ptr;
}

template <class MemDepPred, class Impl>
void
MemDepUnit<MemDepPred, Impl>::insert(DynInstPtr &inst)
{
    unsigned tid = inst->threadNumber;

    MemDepEntryPtr inst_entry = new MemDepEntry(inst);

    // Add the MemDepEntry to the hash.
    memDepHash.insert(
        std::pair<InstSeqNum, MemDepEntryPtr>(inst->seqNum, inst_entry));
#ifdef DEBUG
    MemDepEntry::memdep_insert++;
#endif

    instList[tid].push_back(inst);

    inst_entry->listIt = --(instList[tid].end());

    // Check any barriers and the dependence predictor for any
    // producing memrefs/stores.
    InstSeqNum producing_store;
    if (inst->isLoad() && loadBarrier) {
        DPRINTF(MemDepUnit, "Load barrier [sn:%lli] in flight\n",
                loadBarrierSN);
        producing_store = loadBarrierSN;
    } else if (inst->isStore() && storeBarrier) {
        DPRINTF(MemDepUnit, "Store barrier [sn:%lli] in flight\n",
                storeBarrierSN);
        producing_store = storeBarrierSN;
    } else {
        producing_store = depPred.checkInst(inst->readPC());
    }

    MemDepEntryPtr store_entry = NULL;

    // If there is a producing store, try to find the entry.
    if (producing_store != 0) {
        DPRINTF(MemDepUnit, "Searching for producer\n");
        MemDepHashIt hash_it = memDepHash.find(producing_store);

        if (hash_it != memDepHash.end()) {
            store_entry = (*hash_it).second;
            DPRINTF(MemDepUnit, "Proucer found\n");
        }
    }

    // If no store entry, then instruction can issue as soon as the registers
    // are ready.
    if (!store_entry) {
        DPRINTF(MemDepUnit, "No dependency for inst PC "
                "%#x [sn:%lli].\n", inst->readPC(), inst->seqNum);

        inst_entry->memDepReady = true;

        if (inst->readyToIssue()) {
            inst_entry->regsReady = true;

            moveToReady(inst_entry);
        }
    } else {
        // Otherwise make the instruction dependent on the store/barrier.
        DPRINTF(MemDepUnit, "Adding to dependency list; "
                "inst PC %#x is dependent on [sn:%lli].\n",
                inst->readPC(), producing_store);

        if (inst->readyToIssue()) {
            inst_entry->regsReady = true;
        }

        // Clear the bit saying this instruction can issue.
        inst->clearCanIssue();

        // Add this instruction to the list of dependents.
        store_entry->dependInsts.push_back(inst_entry);

        if (inst->isLoad()) {
            ++conflictingLoads;
        } else {
            ++conflictingStores;
        }
    }

    if (inst->isStore()) {
        DPRINTF(MemDepUnit, "Inserting store PC %#x [sn:%lli].\n",
                inst->readPC(), inst->seqNum);

        depPred.insertStore(inst->readPC(), inst->seqNum, inst->threadNumber);

        ++insertedStores;
    } else if (inst->isLoad()) {
        ++insertedLoads;
    } else {
        panic("Unknown type! (most likely a barrier).");
    }
}

template <class MemDepPred, class Impl>
void
MemDepUnit<MemDepPred, Impl>::insertNonSpec(DynInstPtr &inst)
{
    unsigned tid = inst->threadNumber;

    MemDepEntryPtr inst_entry = new MemDepEntry(inst);

    // Insert the MemDepEntry into the hash.
    memDepHash.insert(
        std::pair<InstSeqNum, MemDepEntryPtr>(inst->seqNum, inst_entry));
#ifdef DEBUG
    MemDepEntry::memdep_insert++;
#endif

    // Add the instruction to the list.
    instList[tid].push_back(inst);

    inst_entry->listIt = --(instList[tid].end());

    // Might want to turn this part into an inline function or something.
    // It's shared between both insert functions.
    if (inst->isStore()) {
        DPRINTF(MemDepUnit, "Inserting store PC %#x [sn:%lli].\n",
                inst->readPC(), inst->seqNum);

        depPred.insertStore(inst->readPC(), inst->seqNum, inst->threadNumber);

        ++insertedStores;
    } else if (inst->isLoad()) {
        ++insertedLoads;
    } else {
        panic("Unknown type! (most likely a barrier).");
    }
}

template <class MemDepPred, class Impl>
void
MemDepUnit<MemDepPred, Impl>::insertBarrier(DynInstPtr &barr_inst)
{
    InstSeqNum barr_sn = barr_inst->seqNum;
    // Memory barriers block loads and stores, write barriers only stores.
    if (barr_inst->isMemBarrier()) {
        loadBarrier = true;
        loadBarrierSN = barr_sn;
        storeBarrier = true;
        storeBarrierSN = barr_sn;
        DPRINTF(MemDepUnit, "Inserted a memory barrier\n");
    } else if (barr_inst->isWriteBarrier()) {
        storeBarrier = true;
        storeBarrierSN = barr_sn;
        DPRINTF(MemDepUnit, "Inserted a write barrier\n");
    }

    unsigned tid = barr_inst->threadNumber;

    MemDepEntryPtr inst_entry = new MemDepEntry(barr_inst);

    // Add the MemDepEntry to the hash.
    memDepHash.insert(
        std::pair<InstSeqNum, MemDepEntryPtr>(barr_sn, inst_entry));
#ifdef DEBUG
    MemDepEntry::memdep_insert++;
#endif

    // Add the instruction to the instruction list.
    instList[tid].push_back(barr_inst);

    inst_entry->listIt = --(instList[tid].end());
}

template <class MemDepPred, class Impl>
void
MemDepUnit<MemDepPred, Impl>::regsReady(DynInstPtr &inst)
{
    DPRINTF(MemDepUnit, "Marking registers as ready for "
            "instruction PC %#x [sn:%lli].\n",
            inst->readPC(), inst->seqNum);

    MemDepEntryPtr inst_entry = findInHash(inst);

    inst_entry->regsReady = true;

    if (inst_entry->memDepReady) {
        DPRINTF(MemDepUnit, "Instruction has its memory "
                "dependencies resolved, adding it to the ready list.\n");

        moveToReady(inst_entry);
    } else {
        DPRINTF(MemDepUnit, "Instruction still waiting on "
                "memory dependency.\n");
    }
}

template <class MemDepPred, class Impl>
void
MemDepUnit<MemDepPred, Impl>::nonSpecInstReady(DynInstPtr &inst)
{
    DPRINTF(MemDepUnit, "Marking non speculative "
            "instruction PC %#x as ready [sn:%lli].\n",
            inst->readPC(), inst->seqNum);

    MemDepEntryPtr inst_entry = findInHash(inst);

    moveToReady(inst_entry);
}

template <class MemDepPred, class Impl>
void
MemDepUnit<MemDepPred, Impl>::reschedule(DynInstPtr &inst)
{
    instsToReplay.push_back(inst);
}

template <class MemDepPred, class Impl>
void
MemDepUnit<MemDepPred, Impl>::replay(DynInstPtr &inst)
{
    DynInstPtr temp_inst;

    // For now this replay function replays all waiting memory ops.
    while (!instsToReplay.empty()) {
        temp_inst = instsToReplay.front();

        MemDepEntryPtr inst_entry = findInHash(temp_inst);

        DPRINTF(MemDepUnit, "Replaying mem instruction PC %#x "
                "[sn:%lli].\n",
                temp_inst->readPC(), temp_inst->seqNum);

        moveToReady(inst_entry);

        instsToReplay.pop_front();
    }
}

template <class MemDepPred, class Impl>
void
MemDepUnit<MemDepPred, Impl>::completed(DynInstPtr &inst)
{
    DPRINTF(MemDepUnit, "Completed mem instruction PC %#x "
            "[sn:%lli].\n",
            inst->readPC(), inst->seqNum);

    unsigned tid = inst->threadNumber;

    // Remove the instruction from the hash and the list.
    MemDepHashIt hash_it = memDepHash.find(inst->seqNum);

    assert(hash_it != memDepHash.end());

    instList[tid].erase((*hash_it).second->listIt);

    (*hash_it).second = NULL;

    memDepHash.erase(hash_it);
#ifdef DEBUG
    MemDepEntry::memdep_erase++;
#endif
}

template <class MemDepPred, class Impl>
void
MemDepUnit<MemDepPred, Impl>::completeBarrier(DynInstPtr &inst)
{
    wakeDependents(inst);
    completed(inst);

    InstSeqNum barr_sn = inst->seqNum;

    if (inst->isMemBarrier()) {
        assert(loadBarrier && storeBarrier);
        if (loadBarrierSN == barr_sn)
            loadBarrier = false;
        if (storeBarrierSN == barr_sn)
            storeBarrier = false;
    } else if (inst->isWriteBarrier()) {
        assert(storeBarrier);
        if (storeBarrierSN == barr_sn)
            storeBarrier = false;
    }
}

template <class MemDepPred, class Impl>
void
MemDepUnit<MemDepPred, Impl>::wakeDependents(DynInstPtr &inst)
{
    // Only stores and barriers have dependents.
    if (!inst->isStore() && !inst->isMemBarrier() && !inst->isWriteBarrier()) {
        return;
    }

    MemDepEntryPtr inst_entry = findInHash(inst);

    for (int i = 0; i < inst_entry->dependInsts.size(); ++i ) {
        MemDepEntryPtr woken_inst = inst_entry->dependInsts[i];

        if (!woken_inst->inst) {
            // Potentially removed mem dep entries could be on this list
            continue;
        }

        DPRINTF(MemDepUnit, "Waking up a dependent inst, "
                "[sn:%lli].\n",
                woken_inst->inst->seqNum);

        if (woken_inst->regsReady && !woken_inst->squashed) {
            moveToReady(woken_inst);
        } else {
            woken_inst->memDepReady = true;
        }
    }

    inst_entry->dependInsts.clear();
}

template <class MemDepPred, class Impl>
void
MemDepUnit<MemDepPred, Impl>::squash(const InstSeqNum &squashed_num,
                                     unsigned tid)
{
    if (!instsToReplay.empty()) {
        ListIt replay_it = instsToReplay.begin();
        while (replay_it != instsToReplay.end()) {
            if ((*replay_it)->threadNumber == tid &&
                (*replay_it)->seqNum > squashed_num) {
                instsToReplay.erase(replay_it++);
            } else {
                ++replay_it;
            }
        }
    }

    ListIt squash_it = instList[tid].end();
    --squash_it;

    MemDepHashIt hash_it;

    while (!instList[tid].empty() &&
           (*squash_it)->seqNum > squashed_num) {

        DPRINTF(MemDepUnit, "Squashing inst [sn:%lli]\n",
                (*squash_it)->seqNum);

        hash_it = memDepHash.find((*squash_it)->seqNum);

        assert(hash_it != memDepHash.end());

        (*hash_it).second->squashed = true;

        (*hash_it).second = NULL;

        memDepHash.erase(hash_it);
#ifdef DEBUG
        MemDepEntry::memdep_erase++;
#endif

        instList[tid].erase(squash_it--);
    }

    // Tell the dependency predictor to squash as well.
    depPred.squash(squashed_num, tid);
}

template <class MemDepPred, class Impl>
void
MemDepUnit<MemDepPred, Impl>::violation(DynInstPtr &store_inst,
                                        DynInstPtr &violating_load)
{
    DPRINTF(MemDepUnit, "Passing violating PCs to store sets,"
            " load: %#x, store: %#x\n", violating_load->readPC(),
            store_inst->readPC());
    // Tell the memory dependence unit of the violation.
    depPred.violation(violating_load->readPC(), store_inst->readPC());
}

template <class MemDepPred, class Impl>
void
MemDepUnit<MemDepPred, Impl>::issue(DynInstPtr &inst)
{
    DPRINTF(MemDepUnit, "Issuing instruction PC %#x [sn:%lli].\n",
            inst->readPC(), inst->seqNum);

    depPred.issued(inst->readPC(), inst->seqNum, inst->isStore());
}

template <class MemDepPred, class Impl>
inline typename MemDepUnit<MemDepPred,Impl>::MemDepEntryPtr &
MemDepUnit<MemDepPred, Impl>::findInHash(const DynInstPtr &inst)
{
    MemDepHashIt hash_it = memDepHash.find(inst->seqNum);

    assert(hash_it != memDepHash.end());

    return (*hash_it).second;
}

template <class MemDepPred, class Impl>
inline void
MemDepUnit<MemDepPred, Impl>::moveToReady(MemDepEntryPtr &woken_inst_entry)
{
    DPRINTF(MemDepUnit, "Adding instruction [sn:%lli] "
            "to the ready list.\n", woken_inst_entry->inst->seqNum);

    assert(!woken_inst_entry->squashed);

    iqPtr->addReadyMemInst(woken_inst_entry->inst);
}


template <class MemDepPred, class Impl>
void
MemDepUnit<MemDepPred, Impl>::dumpLists()
{
    for (unsigned tid=0; tid < Impl::MaxThreads; tid++) {
        cprintf("Instruction list %i size: %i\n",
                tid, instList[tid].size());

        ListIt inst_list_it = instList[tid].begin();
        int num = 0;

        while (inst_list_it != instList[tid].end()) {
            cprintf("Instruction:%i\nPC:%#x\n[sn:%i]\n[tid:%i]\nIssued:%i\n"
                    "Squashed:%i\n\n",
                    num, (*inst_list_it)->readPC(),
                    (*inst_list_it)->seqNum,
                    (*inst_list_it)->threadNumber,
                    (*inst_list_it)->isIssued(),
                    (*inst_list_it)->isSquashed());
            inst_list_it++;
            ++num;
        }
    }

    cprintf("Memory dependence hash size: %i\n", memDepHash.size());

#ifdef DEBUG
    cprintf("Memory dependence entries: %i\n", MemDepEntry::memdep_count);
#endif
}