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

#include "arch/locked_mem.hh"
#include "arch/mmaped_ipr.hh"
#include "arch/utility.hh"
#include "base/bigint.hh"
#include "cpu/exetrace.hh"
#include "cpu/simple/timing.hh"
#include "mem/packet.hh"
#include "mem/packet_access.hh"
#include "params/TimingSimpleCPU.hh"
#include "sim/system.hh"

using namespace std;
using namespace TheISA;

Port *
TimingSimpleCPU::getPort(const std::string &if_name, int idx)
{
    if (if_name == "dcache_port")
        return &dcachePort;
    else if (if_name == "icache_port")
        return &icachePort;
    else
        panic("No Such Port\n");
}

void
TimingSimpleCPU::init()
{
    BaseCPU::init();
    cpuId = tc->readCpuId();
#if FULL_SYSTEM
    for (int i = 0; i < threadContexts.size(); ++i) {
        ThreadContext *tc = threadContexts[i];

        // initialize CPU, including PC
        TheISA::initCPU(tc, cpuId);
    }
#endif
}

Tick
TimingSimpleCPU::CpuPort::recvAtomic(PacketPtr pkt)
{
    panic("TimingSimpleCPU doesn't expect recvAtomic callback!");
    return curTick;
}

void
TimingSimpleCPU::CpuPort::recvFunctional(PacketPtr pkt)
{
    //No internal storage to update, jusst return
    return;
}

void
TimingSimpleCPU::CpuPort::recvStatusChange(Status status)
{
    if (status == RangeChange) {
        if (!snoopRangeSent) {
            snoopRangeSent = true;
            sendStatusChange(Port::RangeChange);
        }
        return;
    }

    panic("TimingSimpleCPU doesn't expect recvStatusChange callback!");
}


void
TimingSimpleCPU::CpuPort::TickEvent::schedule(PacketPtr _pkt, Tick t)
{
    pkt = _pkt;
    Event::schedule(t);
}

TimingSimpleCPU::TimingSimpleCPU(Params *p)
    : BaseSimpleCPU(p), icachePort(this, p->clock), dcachePort(this, p->clock)
{
    _status = Idle;

    icachePort.snoopRangeSent = false;
    dcachePort.snoopRangeSent = false;

    ifetch_pkt = dcache_pkt = NULL;
    drainEvent = NULL;
    fetchEvent = NULL;
    previousTick = 0;
    changeState(SimObject::Running);
}


TimingSimpleCPU::~TimingSimpleCPU()
{
}

void
TimingSimpleCPU::serialize(ostream &os)
{
    SimObject::State so_state = SimObject::getState();
    SERIALIZE_ENUM(so_state);
    BaseSimpleCPU::serialize(os);
}

void
TimingSimpleCPU::unserialize(Checkpoint *cp, const string &section)
{
    SimObject::State so_state;
    UNSERIALIZE_ENUM(so_state);
    BaseSimpleCPU::unserialize(cp, section);
}

unsigned int
TimingSimpleCPU::drain(Event *drain_event)
{
    // TimingSimpleCPU is ready to drain if it's not waiting for
    // an access to complete.
    if (status() == Idle || status() == Running || status() == SwitchedOut) {
        changeState(SimObject::Drained);
        return 0;
    } else {
        changeState(SimObject::Draining);
        drainEvent = drain_event;
        return 1;
    }
}

void
TimingSimpleCPU::resume()
{
    DPRINTF(SimpleCPU, "Resume\n");
    if (_status != SwitchedOut && _status != Idle) {
        assert(system->getMemoryMode() == Enums::timing);

        // Delete the old event if it existed.
        if (fetchEvent) {
            if (fetchEvent->scheduled())
                fetchEvent->deschedule();

            delete fetchEvent;
        }

        fetchEvent = new FetchEvent(this, nextCycle());
    }

    changeState(SimObject::Running);
}

void
TimingSimpleCPU::switchOut()
{
    assert(status() == Running || status() == Idle);
    _status = SwitchedOut;
    numCycles += tickToCycles(curTick - previousTick);

    // If we've been scheduled to resume but are then told to switch out,
    // we'll need to cancel it.
    if (fetchEvent && fetchEvent->scheduled())
        fetchEvent->deschedule();
}


void
TimingSimpleCPU::takeOverFrom(BaseCPU *oldCPU)
{
    BaseCPU::takeOverFrom(oldCPU, &icachePort, &dcachePort);

    // if any of this CPU's ThreadContexts are active, mark the CPU as
    // running and schedule its tick event.
    for (int i = 0; i < threadContexts.size(); ++i) {
        ThreadContext *tc = threadContexts[i];
        if (tc->status() == ThreadContext::Active && _status != Running) {
            _status = Running;
            break;
        }
    }

    if (_status != Running) {
        _status = Idle;
    }
    assert(threadContexts.size() == 1);
    cpuId = tc->readCpuId(); 
    previousTick = curTick;
}


void
TimingSimpleCPU::activateContext(int thread_num, int delay)
{
    DPRINTF(SimpleCPU, "ActivateContext %d (%d cycles)\n", thread_num, delay);

    assert(thread_num == 0);
    assert(thread);

    assert(_status == Idle);

    notIdleFraction++;
    _status = Running;

    // kick things off by initiating the fetch of the next instruction
    fetchEvent = new FetchEvent(this, nextCycle(curTick + ticks(delay)));
}


void
TimingSimpleCPU::suspendContext(int thread_num)
{
    DPRINTF(SimpleCPU, "SuspendContext %d\n", thread_num);

    assert(thread_num == 0);
    assert(thread);

    assert(_status == Running);

    // just change status to Idle... if status != Running,
    // completeInst() will not initiate fetch of next instruction.

    notIdleFraction--;
    _status = Idle;
}


template <class T>
Fault
TimingSimpleCPU::read(Addr addr, T &data, unsigned flags)
{
    Request *req =
        new Request(/* asid */ 0, addr, sizeof(T), flags, thread->readPC(),
                    cpuId, /* thread ID */ 0);

    if (traceData) {
        traceData->setAddr(req->getVaddr());
    }

   // translate to physical address
    Fault fault = thread->translateDataReadReq(req);

    // Now do the access.
    if (fault == NoFault) {
        PacketPtr pkt =
            new Packet(req,
                       (req->isLocked() ?
                        MemCmd::LoadLockedReq : MemCmd::ReadReq),
                       Packet::Broadcast);
        pkt->dataDynamic<T>(new T);

        if (req->isMmapedIpr()) {
            Tick delay;
            delay = TheISA::handleIprRead(thread->getTC(), pkt);
            new IprEvent(pkt, this, nextCycle(curTick + delay));
            _status = DcacheWaitResponse;
            dcache_pkt = NULL;
        } else if (!dcachePort.sendTiming(pkt)) {
            _status = DcacheRetry;
            dcache_pkt = pkt;
        } else {
            _status = DcacheWaitResponse;
            // memory system takes ownership of packet
            dcache_pkt = NULL;
        }

        // This will need a new way to tell if it has a dcache attached.
        if (req->isUncacheable())
            recordEvent("Uncached Read");
    } else {
        delete req;
    }

    return fault;
}

Fault
TimingSimpleCPU::translateDataReadAddr(Addr vaddr, Addr &paddr,
        int size, unsigned flags)
{
    Request *req =
        new Request(0, vaddr, size, flags, thread->readPC(), cpuId, 0);

    if (traceData) {
        traceData->setAddr(vaddr);
    }

    Fault fault = thread->translateDataWriteReq(req);

    if (fault == NoFault)
        paddr = req->getPaddr();

    delete req;
    return fault;
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS

template
Fault
TimingSimpleCPU::read(Addr addr, Twin64_t &data, unsigned flags);

template
Fault
TimingSimpleCPU::read(Addr addr, Twin32_t &data, unsigned flags);

template
Fault
TimingSimpleCPU::read(Addr addr, uint64_t &data, unsigned flags);

template
Fault
TimingSimpleCPU::read(Addr addr, uint32_t &data, unsigned flags);

template
Fault
TimingSimpleCPU::read(Addr addr, uint16_t &data, unsigned flags);

template
Fault
TimingSimpleCPU::read(Addr addr, uint8_t &data, unsigned flags);

#endif //DOXYGEN_SHOULD_SKIP_THIS

template<>
Fault
TimingSimpleCPU::read(Addr addr, double &data, unsigned flags)
{
    return read(addr, *(uint64_t*)&data, flags);
}

template<>
Fault
TimingSimpleCPU::read(Addr addr, float &data, unsigned flags)
{
    return read(addr, *(uint32_t*)&data, flags);
}


template<>
Fault
TimingSimpleCPU::read(Addr addr, int32_t &data, unsigned flags)
{
    return read(addr, (uint32_t&)data, flags);
}


template <class T>
Fault
TimingSimpleCPU::write(T data, Addr addr, unsigned flags, uint64_t *res)
{
    Request *req =
        new Request(/* asid */ 0, addr, sizeof(T), flags, thread->readPC(),
                    cpuId, /* thread ID */ 0);

    if (traceData) {
        traceData->setAddr(req->getVaddr());
    }

    // translate to physical address
    Fault fault = thread->translateDataWriteReq(req);

    // Now do the access.
    if (fault == NoFault) {
        MemCmd cmd = MemCmd::WriteReq; // default
        bool do_access = true;  // flag to suppress cache access

        if (req->isLocked()) {
            cmd = MemCmd::StoreCondReq;
            do_access = TheISA::handleLockedWrite(thread, req);
        } else if (req->isSwap()) {
            cmd = MemCmd::SwapReq;
            if (req->isCondSwap()) {
                assert(res);
                req->setExtraData(*res);
            }
        }

        // Note: need to allocate dcache_pkt even if do_access is
        // false, as it's used unconditionally to call completeAcc().
        assert(dcache_pkt == NULL);
        dcache_pkt = new Packet(req, cmd, Packet::Broadcast);
        dcache_pkt->allocate();
        dcache_pkt->set(data);

        if (do_access) {
            if (req->isMmapedIpr()) {
                Tick delay;
                dcache_pkt->set(htog(data));
                delay = TheISA::handleIprWrite(thread->getTC(), dcache_pkt);
                new IprEvent(dcache_pkt, this, nextCycle(curTick + delay));
                _status = DcacheWaitResponse;
                dcache_pkt = NULL;
            } else if (!dcachePort.sendTiming(dcache_pkt)) {
                _status = DcacheRetry;
            } else {
                _status = DcacheWaitResponse;
                // memory system takes ownership of packet
                dcache_pkt = NULL;
            }
        }
        // This will need a new way to tell if it's hooked up to a cache or not.
        if (req->isUncacheable())
            recordEvent("Uncached Write");
    } else {
        delete req;
    }


    // If the write needs to have a fault on the access, consider calling
    // changeStatus() and changing it to "bad addr write" or something.
    return fault;
}

Fault
TimingSimpleCPU::translateDataWriteAddr(Addr vaddr, Addr &paddr,
        int size, unsigned flags)
{
    Request *req =
        new Request(0, vaddr, size, flags, thread->readPC(), cpuId, 0);

    if (traceData) {
        traceData->setAddr(vaddr);
    }

    Fault fault = thread->translateDataWriteReq(req);

    if (fault == NoFault)
        paddr = req->getPaddr();

    delete req;
    return fault;
}


#ifndef DOXYGEN_SHOULD_SKIP_THIS
template
Fault
TimingSimpleCPU::write(Twin32_t data, Addr addr,
                       unsigned flags, uint64_t *res);

template
Fault
TimingSimpleCPU::write(Twin64_t data, Addr addr,
                       unsigned flags, uint64_t *res);

template
Fault
TimingSimpleCPU::write(uint64_t data, Addr addr,
                       unsigned flags, uint64_t *res);

template
Fault
TimingSimpleCPU::write(uint32_t data, Addr addr,
                       unsigned flags, uint64_t *res);

template
Fault
TimingSimpleCPU::write(uint16_t data, Addr addr,
                       unsigned flags, uint64_t *res);

template
Fault
TimingSimpleCPU::write(uint8_t data, Addr addr,
                       unsigned flags, uint64_t *res);

#endif //DOXYGEN_SHOULD_SKIP_THIS

template<>
Fault
TimingSimpleCPU::write(double data, Addr addr, unsigned flags, uint64_t *res)
{
    return write(*(uint64_t*)&data, addr, flags, res);
}

template<>
Fault
TimingSimpleCPU::write(float data, Addr addr, unsigned flags, uint64_t *res)
{
    return write(*(uint32_t*)&data, addr, flags, res);
}


template<>
Fault
TimingSimpleCPU::write(int32_t data, Addr addr, unsigned flags, uint64_t *res)
{
    return write((uint32_t)data, addr, flags, res);
}


void
TimingSimpleCPU::fetch()
{
    DPRINTF(SimpleCPU, "Fetch\n");

    if (!curStaticInst || !curStaticInst->isDelayedCommit())
        checkForInterrupts();

    checkPcEventQueue();

    Request *ifetch_req = new Request();
    ifetch_req->setThreadContext(cpuId, /* thread ID */ 0);
    Fault fault = setupFetchRequest(ifetch_req);

    ifetch_pkt = new Packet(ifetch_req, MemCmd::ReadReq, Packet::Broadcast);
    ifetch_pkt->dataStatic(&inst);

    if (fault == NoFault) {
        if (!icachePort.sendTiming(ifetch_pkt)) {
            // Need to wait for retry
            _status = IcacheRetry;
        } else {
            // Need to wait for cache to respond
            _status = IcacheWaitResponse;
            // ownership of packet transferred to memory system
            ifetch_pkt = NULL;
        }
    } else {
        delete ifetch_req;
        delete ifetch_pkt;
        // fetch fault: advance directly to next instruction (fault handler)
        advanceInst(fault);
    }

    numCycles += tickToCycles(curTick - previousTick);
    previousTick = curTick;
}


void
TimingSimpleCPU::advanceInst(Fault fault)
{
    advancePC(fault);

    if (_status == Running) {
        // kick off fetch of next instruction... callback from icache
        // response will cause that instruction to be executed,
        // keeping the CPU running.
        fetch();
    }
}


void
TimingSimpleCPU::completeIfetch(PacketPtr pkt)
{
    DPRINTF(SimpleCPU, "Complete ICache Fetch\n");

    // received a response from the icache: execute the received
    // instruction
    assert(!pkt->isError());
    assert(_status == IcacheWaitResponse);

    _status = Running;

    numCycles += tickToCycles(curTick - previousTick);
    previousTick = curTick;

    if (getState() == SimObject::Draining) {
        delete pkt->req;
        delete pkt;

        completeDrain();
        return;
    }

    preExecute();
    if (curStaticInst->isMemRef() && !curStaticInst->isDataPrefetch()) {
        // load or store: just send to dcache
        Fault fault = curStaticInst->initiateAcc(this, traceData);
        if (_status != Running) {
            // instruction will complete in dcache response callback
            assert(_status == DcacheWaitResponse || _status == DcacheRetry);
            assert(fault == NoFault);
        } else {
            if (fault == NoFault) {
                // Note that ARM can have NULL packets if the instruction gets
                // squashed due to predication
                // early fail on store conditional: complete now
                assert(dcache_pkt != NULL || THE_ISA == ARM_ISA);

                fault = curStaticInst->completeAcc(dcache_pkt, this,
                                                   traceData);
                if (dcache_pkt != NULL)
                {
                    delete dcache_pkt->req;
                    delete dcache_pkt;
                    dcache_pkt = NULL;
                }

                // keep an instruction count
                if (fault == NoFault)
                    countInst();
            } else if (traceData) {
                // If there was a fault, we shouldn't trace this instruction.
                delete traceData;
                traceData = NULL;
            }

            postExecute();
            // @todo remove me after debugging with legion done
            if (curStaticInst && (!curStaticInst->isMicroop() ||
                        curStaticInst->isFirstMicroop()))
                instCnt++;
            advanceInst(fault);
        }
    } else {
        // non-memory instruction: execute completely now
        Fault fault = curStaticInst->execute(this, traceData);

        // keep an instruction count
        if (fault == NoFault)
            countInst();
        else if (traceData) {
            // If there was a fault, we shouldn't trace this instruction.
            delete traceData;
            traceData = NULL;
        }

        postExecute();
        // @todo remove me after debugging with legion done
        if (curStaticInst && (!curStaticInst->isMicroop() ||
                    curStaticInst->isFirstMicroop()))
            instCnt++;
        advanceInst(fault);
    }

    delete pkt->req;
    delete pkt;
}

void
TimingSimpleCPU::IcachePort::ITickEvent::process()
{
    cpu->completeIfetch(pkt);
}

bool
TimingSimpleCPU::IcachePort::recvTiming(PacketPtr pkt)
{
    if (pkt->isResponse() && !pkt->wasNacked()) {
        // delay processing of returned data until next CPU clock edge
        Tick next_tick = cpu->nextCycle(curTick);

        if (next_tick == curTick)
            cpu->completeIfetch(pkt);
        else
            tickEvent.schedule(pkt, next_tick);

        return true;
    }
    else if (pkt->wasNacked()) {
        assert(cpu->_status == IcacheWaitResponse);
        pkt->reinitNacked();
        if (!sendTiming(pkt)) {
            cpu->_status = IcacheRetry;
            cpu->ifetch_pkt = pkt;
        }
    }
    //Snooping a Coherence Request, do nothing
    return true;
}

void
TimingSimpleCPU::IcachePort::recvRetry()
{
    // we shouldn't get a retry unless we have a packet that we're
    // waiting to transmit
    assert(cpu->ifetch_pkt != NULL);
    assert(cpu->_status == IcacheRetry);
    PacketPtr tmp = cpu->ifetch_pkt;
    if (sendTiming(tmp)) {
        cpu->_status = IcacheWaitResponse;
        cpu->ifetch_pkt = NULL;
    }
}

void
TimingSimpleCPU::completeDataAccess(PacketPtr pkt)
{
    // received a response from the dcache: complete the load or store
    // instruction
    assert(!pkt->isError());
    assert(_status == DcacheWaitResponse);
    _status = Running;

    numCycles += tickToCycles(curTick - previousTick);
    previousTick = curTick;

    Fault fault = curStaticInst->completeAcc(pkt, this, traceData);

    // keep an instruction count
    if (fault == NoFault)
        countInst();
    else if (traceData) {
        // If there was a fault, we shouldn't trace this instruction.
        delete traceData;
        traceData = NULL;
    }

    if (pkt->isRead() && pkt->isLocked()) {
        TheISA::handleLockedRead(thread, pkt->req);
    }

    delete pkt->req;
    delete pkt;

    postExecute();

    if (getState() == SimObject::Draining) {
        advancePC(fault);
        completeDrain();

        return;
    }

    advanceInst(fault);
}


void
TimingSimpleCPU::completeDrain()
{
    DPRINTF(Config, "Done draining\n");
    changeState(SimObject::Drained);
    drainEvent->process();
}

void
TimingSimpleCPU::DcachePort::setPeer(Port *port)
{
    Port::setPeer(port);

#if FULL_SYSTEM
    // Update the ThreadContext's memory ports (Functional/Virtual
    // Ports)
    cpu->tcBase()->connectMemPorts();
#endif
}

bool
TimingSimpleCPU::DcachePort::recvTiming(PacketPtr pkt)
{
    if (pkt->isResponse() && !pkt->wasNacked()) {
        // delay processing of returned data until next CPU clock edge
        Tick next_tick = cpu->nextCycle(curTick);

        if (next_tick == curTick)
            cpu->completeDataAccess(pkt);
        else
            tickEvent.schedule(pkt, next_tick);

        return true;
    }
    else if (pkt->wasNacked()) {
        assert(cpu->_status == DcacheWaitResponse);
        pkt->reinitNacked();
        if (!sendTiming(pkt)) {
            cpu->_status = DcacheRetry;
            cpu->dcache_pkt = pkt;
        }
    }
    //Snooping a Coherence Request, do nothing
    return true;
}

void
TimingSimpleCPU::DcachePort::DTickEvent::process()
{
    cpu->completeDataAccess(pkt);
}

void
TimingSimpleCPU::DcachePort::recvRetry()
{
    // we shouldn't get a retry unless we have a packet that we're
    // waiting to transmit
    assert(cpu->dcache_pkt != NULL);
    assert(cpu->_status == DcacheRetry);
    PacketPtr tmp = cpu->dcache_pkt;
    if (sendTiming(tmp)) {
        cpu->_status = DcacheWaitResponse;
        // memory system takes ownership of packet
        cpu->dcache_pkt = NULL;
    }
}

TimingSimpleCPU::IprEvent::IprEvent(Packet *_pkt, TimingSimpleCPU *_cpu, Tick t)
    : Event(&mainEventQueue), pkt(_pkt), cpu(_cpu)
{
    schedule(t);
}

void
TimingSimpleCPU::IprEvent::process()
{
    cpu->completeDataAccess(pkt);
}

const char *
TimingSimpleCPU::IprEvent::description() const
{
    return "Timing Simple CPU Delay IPR event";
}


void
TimingSimpleCPU::printAddr(Addr a)
{
    dcachePort.printAddr(a);
}


////////////////////////////////////////////////////////////////////////
//
//  TimingSimpleCPU Simulation Object
//
TimingSimpleCPU *
TimingSimpleCPUParams::create()
{
    TimingSimpleCPU::Params *params = new TimingSimpleCPU::Params();
    params->name = name;
    params->numberOfThreads = 1;
    params->max_insts_any_thread = max_insts_any_thread;
    params->max_insts_all_threads = max_insts_all_threads;
    params->max_loads_any_thread = max_loads_any_thread;
    params->max_loads_all_threads = max_loads_all_threads;
    params->progress_interval = progress_interval;
    params->deferRegistration = defer_registration;
    params->clock = clock;
    params->phase = phase;
    params->functionTrace = function_trace;
    params->functionTraceStart = function_trace_start;
    params->system = system;
    params->cpu_id = cpu_id;
    params->tracer = tracer;

    params->itb = itb;
    params->dtb = dtb;
#if FULL_SYSTEM
    params->profile = profile;
    params->do_quiesce = do_quiesce;
    params->do_checkpoint_insts = do_checkpoint_insts;
    params->do_statistics_insts = do_statistics_insts;
#else
    if (workload.size() != 1)
        panic("only one workload allowed");
    params->process = workload[0];
#endif

    TimingSimpleCPU *cpu = new TimingSimpleCPU(params);
    return cpu;
}