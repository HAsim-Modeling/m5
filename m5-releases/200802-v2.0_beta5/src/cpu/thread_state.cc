/*
 * Copyright (c) 2006
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

#include "base/output.hh"
#include "cpu/base.hh"
#include "cpu/profile.hh"
#include "cpu/thread_state.hh"
#include "mem/port.hh"
#include "mem/translating_port.hh"
#include "sim/serialize.hh"

#if FULL_SYSTEM
#include "arch/kernel_stats.hh"
#include "cpu/quiesce_event.hh"
#include "mem/vport.hh"
#endif

#if FULL_SYSTEM
ThreadState::ThreadState(BaseCPU *cpu, int _cpuId, int _tid)
    : baseCpu(cpu), cpuId(_cpuId), tid(_tid), lastActivate(0), lastSuspend(0),
      profile(NULL), profileNode(NULL), profilePC(0), quiesceEvent(NULL),
      physPort(NULL), virtPort(NULL),
      microPC(0), nextMicroPC(1), funcExeInst(0), storeCondFailures(0)
#else
ThreadState::ThreadState(BaseCPU *cpu, int _cpuId, int _tid, Process *_process,
                         short _asid)
    : baseCpu(cpu), cpuId(_cpuId), tid(_tid), lastActivate(0), lastSuspend(0),
      port(NULL), process(_process), asid(_asid),
      microPC(0), nextMicroPC(1), funcExeInst(0), storeCondFailures(0)
#endif
{
    numInst = 0;
    numLoad = 0;
}

ThreadState::~ThreadState()
{
#if !FULL_SYSTEM
    if (port) {
        delete port->getPeer();
        delete port;
    }
#endif
}

void
ThreadState::serialize(std::ostream &os)
{
    SERIALIZE_ENUM(_status);
    // thread_num and cpu_id are deterministic from the config
    SERIALIZE_SCALAR(funcExeInst);
    SERIALIZE_SCALAR(inst);
    SERIALIZE_SCALAR(microPC);
    SERIALIZE_SCALAR(nextMicroPC);

#if FULL_SYSTEM
    Tick quiesceEndTick = 0;
    if (quiesceEvent->scheduled())
        quiesceEndTick = quiesceEvent->when();
    SERIALIZE_SCALAR(quiesceEndTick);
    if (kernelStats)
        kernelStats->serialize(os);
#endif
}

void
ThreadState::unserialize(Checkpoint *cp, const std::string &section)
{

    UNSERIALIZE_ENUM(_status);
    // thread_num and cpu_id are deterministic from the config
    UNSERIALIZE_SCALAR(funcExeInst);
    UNSERIALIZE_SCALAR(inst);
    UNSERIALIZE_SCALAR(microPC);
    UNSERIALIZE_SCALAR(nextMicroPC);

#if FULL_SYSTEM
    Tick quiesceEndTick;
    UNSERIALIZE_SCALAR(quiesceEndTick);
    if (quiesceEndTick)
        quiesceEvent->schedule(quiesceEndTick);
    if (kernelStats)
        kernelStats->unserialize(cp, section);
#endif
}

#if FULL_SYSTEM
void
ThreadState::connectMemPorts()
{
    connectPhysPort();
    connectVirtPort();
}

void
ThreadState::connectPhysPort()
{
    // @todo: For now this disregards any older port that may have
    // already existed.  Fix this memory leak once the bus port IDs
    // for functional ports is resolved.
    if (physPort)
        physPort->removeConn();
    else
        physPort = new FunctionalPort(csprintf("%s-%d-funcport",
                                           baseCpu->name(), tid));
    connectToMemFunc(physPort);
}

void
ThreadState::connectVirtPort()
{
    // @todo: For now this disregards any older port that may have
    // already existed.  Fix this memory leak once the bus port IDs
    // for functional ports is resolved.
    if (virtPort)
        virtPort->removeConn();
    else
        virtPort = new VirtualPort(csprintf("%s-%d-vport",
                                        baseCpu->name(), tid));
    connectToMemFunc(virtPort);
}

void
ThreadState::profileClear()
{
    if (profile)
        profile->clear();
}

void
ThreadState::profileSample()
{
    if (profile)
        profile->sample(profileNode, profilePC);
}

#else
TranslatingPort *
ThreadState::getMemPort()
{
    if (port != NULL)
        return port;

    /* Use this port to for syscall emulation writes to memory. */
    port = new TranslatingPort(csprintf("%s-%d-funcport", baseCpu->name(), tid),
                               process, TranslatingPort::NextPage);

    connectToMemFunc(port);

    return port;
}
#endif

void
ThreadState::connectToMemFunc(Port *port)
{
    Port *dcache_port, *func_mem_port;

    dcache_port = baseCpu->getPort("dcache_port");
    assert(dcache_port != NULL);

    MemObject *mem_object = dcache_port->getPeer()->getOwner();
    assert(mem_object != NULL);

    func_mem_port = mem_object->getPort("functional");
    assert(func_mem_port != NULL);

    func_mem_port->setPeer(port);
    port->setPeer(func_mem_port);
}
