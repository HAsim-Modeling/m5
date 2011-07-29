/*
 * Copyright (c) 2002, 2003, 2004, 2005
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
 */

#include "arch/utility.hh"
#include "arch/faults.hh"
#include "base/cprintf.hh"
#include "base/inifile.hh"
#include "base/loader/symtab.hh"
#include "base/misc.hh"
#include "base/pollevent.hh"
#include "base/range.hh"
#include "base/stats/events.hh"
#include "base/trace.hh"
#include "cpu/base.hh"
#include "cpu/exetrace.hh"
#include "cpu/profile.hh"
#include "cpu/simple/base.hh"
#include "cpu/simple_thread.hh"
#include "cpu/smt.hh"
#include "cpu/static_inst.hh"
#include "cpu/thread_context.hh"
#include "mem/packet.hh"
#include "sim/byteswap.hh"
#include "sim/debug.hh"
#include "sim/host.hh"
#include "sim/sim_events.hh"
#include "sim/sim_object.hh"
#include "sim/stats.hh"
#include "sim/system.hh"

#if FULL_SYSTEM
#include "arch/kernel_stats.hh"
#include "arch/stacktrace.hh"
#include "arch/tlb.hh"
#include "arch/vtophys.hh"
#include "base/remote_gdb.hh"
#else // !FULL_SYSTEM
#include "mem/mem_object.hh"
#endif // FULL_SYSTEM

using namespace std;
using namespace TheISA;

BaseSimpleCPU::BaseSimpleCPU(Params *p)
    : BaseCPU(p), traceData(NULL), thread(NULL), predecoder(NULL)
{
#if FULL_SYSTEM
    thread = new SimpleThread(this, 0, p->system, p->itb, p->dtb);
#else
    thread = new SimpleThread(this, /* thread_num */ 0, p->process,
            p->itb, p->dtb, /* asid */ 0);
#endif // !FULL_SYSTEM

    thread->setStatus(ThreadContext::Unallocated);

    tc = thread->getTC();

    numInst = 0;
    startNumInst = 0;
    numLoad = 0;
    startNumLoad = 0;
    lastIcacheStall = 0;
    lastDcacheStall = 0;

    threadContexts.push_back(tc);


    fetchOffset = 0;
    stayAtPC = false;
}

BaseSimpleCPU::~BaseSimpleCPU()
{
}

void
BaseSimpleCPU::deallocateContext(int thread_num)
{
    // for now, these are equivalent
    suspendContext(thread_num);
}


void
BaseSimpleCPU::haltContext(int thread_num)
{
    // for now, these are equivalent
    suspendContext(thread_num);
}


void
BaseSimpleCPU::regStats()
{
    using namespace Stats;

    BaseCPU::regStats();

    numInsts
        .name(name() + ".num_insts")
        .desc("Number of instructions executed")
        ;

    numMemRefs
        .name(name() + ".num_refs")
        .desc("Number of memory references")
        ;

    notIdleFraction
        .name(name() + ".not_idle_fraction")
        .desc("Percentage of non-idle cycles")
        ;

    idleFraction
        .name(name() + ".idle_fraction")
        .desc("Percentage of idle cycles")
        ;

    icacheStallCycles
        .name(name() + ".icache_stall_cycles")
        .desc("ICache total stall cycles")
        .prereq(icacheStallCycles)
        ;

    dcacheStallCycles
        .name(name() + ".dcache_stall_cycles")
        .desc("DCache total stall cycles")
        .prereq(dcacheStallCycles)
        ;

    icacheRetryCycles
        .name(name() + ".icache_retry_cycles")
        .desc("ICache total retry cycles")
        .prereq(icacheRetryCycles)
        ;

    dcacheRetryCycles
        .name(name() + ".dcache_retry_cycles")
        .desc("DCache total retry cycles")
        .prereq(dcacheRetryCycles)
        ;

    idleFraction = constant(1.0) - notIdleFraction;
}

void
BaseSimpleCPU::resetStats()
{
//    startNumInst = numInst;
    // notIdleFraction = (_status != Idle);
}

void
BaseSimpleCPU::serialize(ostream &os)
{
    BaseCPU::serialize(os);
//    SERIALIZE_SCALAR(inst);
    nameOut(os, csprintf("%s.xc.0", name()));
    thread->serialize(os);
}

void
BaseSimpleCPU::unserialize(Checkpoint *cp, const string &section)
{
    BaseCPU::unserialize(cp, section);
//    UNSERIALIZE_SCALAR(inst);
    thread->unserialize(cp, csprintf("%s.xc.0", section));
}

void
change_thread_state(int thread_number, int activate, int priority)
{
}

Fault
BaseSimpleCPU::copySrcTranslate(Addr src)
{
#if 0
    static bool no_warn = true;
    int blk_size = (dcacheInterface) ? dcacheInterface->getBlockSize() : 64;
    // Only support block sizes of 64 atm.
    assert(blk_size == 64);
    int offset = src & (blk_size - 1);

    // Make sure block doesn't span page
    if (no_warn &&
        (src & PageMask) != ((src + blk_size) & PageMask) &&
        (src >> 40) != 0xfffffc) {
        warn("Copied block source spans pages %x.", src);
        no_warn = false;
    }

    memReq->reset(src & ~(blk_size - 1), blk_size);

    // translate to physical address
    Fault fault = thread->translateDataReadReq(req);

    if (fault == NoFault) {
        thread->copySrcAddr = src;
        thread->copySrcPhysAddr = memReq->paddr + offset;
    } else {
        assert(!fault->isAlignmentFault());

        thread->copySrcAddr = 0;
        thread->copySrcPhysAddr = 0;
    }
    return fault;
#else
    return NoFault;
#endif
}

Fault
BaseSimpleCPU::copy(Addr dest)
{
#if 0
    static bool no_warn = true;
    int blk_size = (dcacheInterface) ? dcacheInterface->getBlockSize() : 64;
    // Only support block sizes of 64 atm.
    assert(blk_size == 64);
    uint8_t data[blk_size];
    //assert(thread->copySrcAddr);
    int offset = dest & (blk_size - 1);

    // Make sure block doesn't span page
    if (no_warn &&
        (dest & PageMask) != ((dest + blk_size) & PageMask) &&
        (dest >> 40) != 0xfffffc) {
        no_warn = false;
        warn("Copied block destination spans pages %x. ", dest);
    }

    memReq->reset(dest & ~(blk_size -1), blk_size);
    // translate to physical address
    Fault fault = thread->translateDataWriteReq(req);

    if (fault == NoFault) {
        Addr dest_addr = memReq->paddr + offset;
        // Need to read straight from memory since we have more than 8 bytes.
        memReq->paddr = thread->copySrcPhysAddr;
        thread->mem->read(memReq, data);
        memReq->paddr = dest_addr;
        thread->mem->write(memReq, data);
        if (dcacheInterface) {
            memReq->cmd = Copy;
            memReq->completionEvent = NULL;
            memReq->paddr = thread->copySrcPhysAddr;
            memReq->dest = dest_addr;
            memReq->size = 64;
            memReq->time = curTick;
            memReq->flags &= ~INST_READ;
            dcacheInterface->access(memReq);
        }
    }
    else
        assert(!fault->isAlignmentFault());

    return fault;
#else
    panic("copy not implemented");
    return NoFault;
#endif
}

#if FULL_SYSTEM
Addr
BaseSimpleCPU::dbg_vtophys(Addr addr)
{
    return vtophys(tc, addr);
}
#endif // FULL_SYSTEM

#if FULL_SYSTEM
void
BaseSimpleCPU::post_interrupt(int int_num, int index)
{
    BaseCPU::post_interrupt(int_num, index);

    if (thread->status() == ThreadContext::Suspended) {
                DPRINTF(Quiesce,"Suspended Processor awoke\n");
        thread->activate();
    }
}
#endif // FULL_SYSTEM

void
BaseSimpleCPU::checkForInterrupts()
{
#if FULL_SYSTEM
    if (check_interrupts(tc)) {
        Fault interrupt = interrupts.getInterrupt(tc);

        if (interrupt != NoFault) {
            interrupts.updateIntrInfo(tc);
            interrupt->invoke(tc);
        }
    }
#endif
}


Fault
BaseSimpleCPU::setupFetchRequest(Request *req)
{
    Addr threadPC = thread->readPC();

    // set up memory request for instruction fetch
#if ISA_HAS_DELAY_SLOT
    DPRINTF(Fetch,"Fetch: PC:%08p NPC:%08p NNPC:%08p\n",threadPC,
            thread->readNextPC(),thread->readNextNPC());
#else
    DPRINTF(Fetch,"Fetch: PC:%08p NPC:%08p\n",threadPC,
            thread->readNextPC());
#endif

    Addr fetchPC = (threadPC & PCMask) + fetchOffset;
    req->setVirt(0, fetchPC, sizeof(MachInst), 0, threadPC);

    Fault fault = thread->translateInstReq(req);

    return fault;
}


void
BaseSimpleCPU::preExecute()
{
    // maintain $r0 semantics
    thread->setIntReg(ZeroReg, 0);
#if THE_ISA == ALPHA_ISA
    thread->setFloatReg(ZeroReg, 0.0);
#endif // ALPHA_ISA

    // check for instruction-count-based events
    comInstEventQueue[0]->serviceEvents(numInst);

    // decode the instruction
    inst = gtoh(inst);

    //If we're not in the middle of a macro instruction
    if (!curMacroStaticInst) {

        StaticInstPtr instPtr = NULL;

        //Predecode, ie bundle up an ExtMachInst
        //This should go away once the constructor can be set up properly
        predecoder.setTC(thread->getTC());
        //If more fetch data is needed, pass it in.
        Addr fetchPC = (thread->readPC() & PCMask) + fetchOffset;
        //if(predecoder.needMoreBytes())
            predecoder.moreBytes(thread->readPC(), fetchPC, inst);
        //else
        //    predecoder.process();

        //If an instruction is ready, decode it. Otherwise, we'll have to
        //fetch beyond the MachInst at the current pc.
        if (predecoder.extMachInstReady()) {
            stayAtPC = false;
            instPtr = StaticInst::decode(predecoder.getExtMachInst(),
                                         thread->readPC());
        } else {
            stayAtPC = true;
            fetchOffset += sizeof(MachInst);
        }

        //If we decoded an instruction and it's microcoded, start pulling
        //out micro ops
        if (instPtr && instPtr->isMacroop()) {
            curMacroStaticInst = instPtr;
            curStaticInst = curMacroStaticInst->
                fetchMicroop(thread->readMicroPC());
        } else {
            curStaticInst = instPtr;
        }
    } else {
        //Read the next micro op from the macro op
        curStaticInst = curMacroStaticInst->
            fetchMicroop(thread->readMicroPC());
    }

    //If we decoded an instruction this "tick", record information about it.
    if(curStaticInst)
    {
#if TRACING_ON
        traceData = tracer->getInstRecord(curTick, tc, curStaticInst,
                                         thread->readPC());

        DPRINTF(Decode,"Decode: Decoded %s instruction: 0x%x\n",
                curStaticInst->getName(), curStaticInst->machInst);
#endif // TRACING_ON

#if FULL_SYSTEM
        thread->setInst(inst);
#endif // FULL_SYSTEM
    }
}

void
BaseSimpleCPU::postExecute()
{
#if FULL_SYSTEM
    if (thread->profile && curStaticInst) {
        bool usermode = TheISA::inUserMode(tc);
        thread->profilePC = usermode ? 1 : thread->readPC();
        ProfileNode *node = thread->profile->consume(tc, curStaticInst);
        if (node)
            thread->profileNode = node;
    }
#endif

    if (curStaticInst->isMemRef()) {
        numMemRefs++;
    }

    if (curStaticInst->isLoad()) {
        ++numLoad;
        comLoadEventQueue[0]->serviceEvents(numLoad);
    }

    traceFunctions(thread->readPC());

    if (traceData) {
        traceData->dump();
        delete traceData;
        traceData = NULL;
    }
}


void
BaseSimpleCPU::advancePC(Fault fault)
{
    //Since we're moving to a new pc, zero out the offset
    fetchOffset = 0;
    if (fault != NoFault) {
        curMacroStaticInst = StaticInst::nullStaticInstPtr;
        predecoder.reset();
        thread->setMicroPC(0);
        thread->setNextMicroPC(1);
        fault->invoke(tc);
    } else {
        //If we're at the last micro op for this instruction
        if (curStaticInst && curStaticInst->isLastMicroop()) {
            //We should be working with a macro op
            assert(curMacroStaticInst);
            //Close out this macro op, and clean up the
            //microcode state
            curMacroStaticInst = StaticInst::nullStaticInstPtr;
            thread->setMicroPC(0);
            thread->setNextMicroPC(1);
        }
        //If we're still in a macro op
        if (curMacroStaticInst) {
            //Advance the micro pc
            thread->setMicroPC(thread->readNextMicroPC());
            //Advance the "next" micro pc. Note that there are no delay
            //slots, and micro ops are "word" addressed.
            thread->setNextMicroPC(thread->readNextMicroPC() + 1);
        } else {
            // go to the next instruction
            thread->setPC(thread->readNextPC());
            thread->setNextPC(thread->readNextNPC());
            thread->setNextNPC(thread->readNextNPC() + sizeof(MachInst));
            assert(thread->readNextPC() != thread->readNextNPC());
        }
    }
}

/*Fault
BaseSimpleCPU::CacheOp(uint8_t Op, Addr EffAddr)
{
    // translate to physical address
    Fault fault = NoFault;
    int CacheID = Op & 0x3; // Lower 3 bits identify Cache
    int CacheOP = Op >> 2; // Upper 3 bits identify Cache Operation
    if(CacheID > 1)
      {
	warn("CacheOps not implemented for secondary/tertiary caches\n");
      }
    else
      {
	switch(CacheOP)
	  { // Fill Packet Type
	  case 0: warn("Invalidate Cache Op\n");
	    break;
	  case 1: warn("Index Load Tag Cache Op\n");
	    break;
	  case 2: warn("Index Store Tag Cache Op\n");
	    break;
	  case 4: warn("Hit Invalidate Cache Op\n");
	    break;
	  case 5: warn("Fill/Hit Writeback Invalidate Cache Op\n");
	    break;
	  case 6: warn("Hit Writeback\n");
	    break;
	  case 7: warn("Fetch & Lock Cache Op\n");
	    break;
	  default: warn("Unimplemented Cache Op\n");
	  }
      }
    return fault;
}*/