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
 * Authors: Ali G. Saidi
 */

/** @file
 * This device implemetns the niagara I/O bridge chip. It manages incomming
 * interrupts and posts them to the CPU when needed. It holds mask registers and
 * various status registers for CPUs to check what interrupts are pending as
 * well as facilities to send IPIs to other cpus.
 */

#include <cstring>

#include "arch/sparc/isa_traits.hh"
#include "arch/sparc/faults.hh"
#include "base/trace.hh"
#include "cpu/intr_control.hh"
#include "dev/sparc/iob.hh"
#include "dev/platform.hh"
#include "mem/port.hh"
#include "mem/packet_access.hh"
#include "sim/faults.hh"
#include "sim/system.hh"

Iob::Iob(const Params *p)
    : PioDevice(p), ic(p->platform->intrctrl)
{
    iobManAddr = ULL(0x9800000000);
    iobManSize = ULL(0x0100000000);
    iobJBusAddr = ULL(0x9F00000000);
    iobJBusSize = ULL(0x0100000000);
    assert (params()->system->threadContexts.size() <= MaxNiagaraProcs);

    pioDelay = p->pio_latency;

    // Get the interrupt controller from the platform
    ic = platform->intrctrl;

    for (int x = 0; x < NumDeviceIds; ++x) {
        intMan[x].cpu = 0;
        intMan[x].vector = 0;
        intCtl[x].mask = true;
        intCtl[x].pend = false;
    }

}

Tick
Iob::read(PacketPtr pkt)
{

    if (pkt->getAddr() >= iobManAddr && pkt->getAddr() < iobManAddr + iobManSize)
        readIob(pkt);
    else if (pkt->getAddr() >= iobJBusAddr && pkt->getAddr() < iobJBusAddr+iobJBusSize)
        readJBus(pkt);
    else
        panic("Invalid address reached Iob\n");

    pkt->makeAtomicResponse();
    return pioDelay;
}

void
Iob::readIob(PacketPtr pkt)
{
        Addr accessAddr = pkt->getAddr() - iobManAddr;
        int index;
        uint64_t data;

        if (accessAddr >= IntManAddr && accessAddr < IntManAddr + IntManSize) {
            index = (accessAddr - IntManAddr) >> 3;
            data = intMan[index].cpu << 8 | intMan[index].vector << 0;
            pkt->set(data);
            return;
        }

        if (accessAddr >= IntCtlAddr && accessAddr < IntCtlAddr + IntCtlSize) {
            index = (accessAddr - IntManAddr) >> 3;
            data = intCtl[index].mask  ? 1 << 2 : 0 |
                   intCtl[index].pend  ? 1 << 0 : 0;
            pkt->set(data);
            return;
        }

        if (accessAddr == JIntVecAddr) {
            pkt->set(jIntVec);
            return;
        }

        panic("Read to unknown IOB offset 0x%x\n", accessAddr);
}

void
Iob::readJBus(PacketPtr pkt)
{
        Addr accessAddr = pkt->getAddr() - iobJBusAddr;
        int cpuid = pkt->req->getCpuNum();
        int index;
        uint64_t data;




        if (accessAddr >= JIntData0Addr && accessAddr < JIntData1Addr) {
            index = (accessAddr - JIntData0Addr) >> 3;
            pkt->set(jBusData0[index]);
            return;
        }

        if (accessAddr >= JIntData1Addr && accessAddr < JIntDataA0Addr) {
            index = (accessAddr - JIntData1Addr) >> 3;
            pkt->set(jBusData1[index]);
            return;
        }

        if (accessAddr == JIntDataA0Addr) {
            pkt->set(jBusData0[cpuid]);
            return;
        }

        if (accessAddr == JIntDataA1Addr) {
            pkt->set(jBusData1[cpuid]);
            return;
        }

        if (accessAddr >= JIntBusyAddr && accessAddr < JIntBusyAddr + JIntBusySize) {
            index = (accessAddr - JIntBusyAddr) >> 3;
            data = jIntBusy[index].busy ? 1 << 5 : 0 |
                   jIntBusy[index].source;
            pkt->set(data);
            return;
        }
        if (accessAddr == JIntABusyAddr) {
            data = jIntBusy[cpuid].busy ? 1 << 5 : 0 |
                   jIntBusy[cpuid].source;
            pkt->set(data);
            return;
        };

        panic("Read to unknown JBus offset 0x%x\n", accessAddr);
}

Tick
Iob::write(PacketPtr pkt)
{
    if (pkt->getAddr() >= iobManAddr && pkt->getAddr() < iobManAddr + iobManSize)
        writeIob(pkt);
    else if (pkt->getAddr() >= iobJBusAddr && pkt->getAddr() < iobJBusAddr+iobJBusSize)
        writeJBus(pkt);
    else
        panic("Invalid address reached Iob\n");


    pkt->makeAtomicResponse();
    return pioDelay;
}

void
Iob::writeIob(PacketPtr pkt)
{
        Addr accessAddr = pkt->getAddr() - iobManAddr;
        int index;
        uint64_t data;

        if (accessAddr >= IntManAddr && accessAddr < IntManAddr + IntManSize) {
            index = (accessAddr - IntManAddr) >> 3;
            data = pkt->get<uint64_t>();
            intMan[index].cpu = bits(data,12,8);
            intMan[index].vector = bits(data,5,0);
            DPRINTF(Iob, "Wrote IntMan %d cpu %d, vec %d\n", index,
                    intMan[index].cpu, intMan[index].vector);
            return;
        }

        if (accessAddr >= IntCtlAddr && accessAddr < IntCtlAddr + IntCtlSize) {
            index = (accessAddr - IntManAddr) >> 3;
            data = pkt->get<uint64_t>();
            intCtl[index].mask = bits(data,2,2);
            if (bits(data,1,1))
                intCtl[index].pend = false;
            DPRINTF(Iob, "Wrote IntCtl %d pend %d cleared %d\n", index,
                    intCtl[index].pend, bits(data,2,2));
            return;
        }

        if (accessAddr == JIntVecAddr) {
            jIntVec = bits(pkt->get<uint64_t>(), 5,0);
            DPRINTF(Iob, "Wrote jIntVec %d\n", jIntVec);
            return;
        }

        if (accessAddr >= IntVecDisAddr && accessAddr < IntVecDisAddr + IntVecDisSize) {
            Type type;
            int cpu_id;
            int vector;
            index = (accessAddr - IntManAddr) >> 3;
            data = pkt->get<uint64_t>();
            type = (Type)bits(data,17,16);
            cpu_id = bits(data, 12,8);
            vector = bits(data,5,0);
            generateIpi(type,cpu_id, vector);
            return;
        }

        panic("Write to unknown IOB offset 0x%x\n", accessAddr);
}

void
Iob::writeJBus(PacketPtr pkt)
{
        Addr accessAddr = pkt->getAddr() - iobJBusAddr;
        int cpuid = pkt->req->getCpuNum();
        int index;
        uint64_t data;

        if (accessAddr >= JIntBusyAddr && accessAddr < JIntBusyAddr + JIntBusySize) {
            index = (accessAddr - JIntBusyAddr) >> 3;
            data = pkt->get<uint64_t>();
            jIntBusy[index].busy = bits(data,5,5);
            DPRINTF(Iob, "Wrote jIntBusy index %d busy: %d\n", index,
                    jIntBusy[index].busy);
            return;
        }
        if (accessAddr == JIntABusyAddr) {
            data = pkt->get<uint64_t>();
            jIntBusy[cpuid].busy = bits(data,5,5);
            DPRINTF(Iob, "Wrote jIntBusy index %d busy: %d\n", cpuid,
                    jIntBusy[cpuid].busy);
            return;
        };

        panic("Write to unknown JBus offset 0x%x\n", accessAddr);
}

void
Iob::receiveDeviceInterrupt(DeviceId devid)
{
    assert(devid < NumDeviceIds);
    if (intCtl[devid].mask)
        return;
    intCtl[devid].mask = true;
    intCtl[devid].pend = true;
    DPRINTF(Iob, "Receiving Device interrupt: %d for cpu %d vec %d\n",
            devid, intMan[devid].cpu, intMan[devid].vector);
    ic->post(intMan[devid].cpu, SparcISA::IT_INT_VEC, intMan[devid].vector);
}


void
Iob::generateIpi(Type type, int cpu_id, int vector)
{
    SparcISA::SparcFault<SparcISA::PowerOnReset> *por = new SparcISA::PowerOnReset();
    if (cpu_id >= sys->getNumCPUs())
        return;

    switch (type) {
      case 0: // interrupt
        DPRINTF(Iob, "Generating interrupt because of I/O write to cpu: %d vec %d\n",
                cpu_id, vector);
        ic->post(cpu_id, SparcISA::IT_INT_VEC, vector);
        break;
      case 1: // reset
        warn("Sending reset to CPU: %d\n", cpu_id);
        if (vector != por->trapType())
            panic("Don't know how to set non-POR reset to cpu\n");
        por->invoke(sys->threadContexts[cpu_id]);
        sys->threadContexts[cpu_id]->activate();
        break;
      case 2: // idle -- this means stop executing and don't wake on interrupts
        DPRINTF(Iob, "Idling CPU because of I/O write cpu: %d\n", cpu_id);
        sys->threadContexts[cpu_id]->halt();
        break;
      case 3: // resume
        DPRINTF(Iob, "Resuming CPU because of I/O write cpu: %d\n", cpu_id);
        sys->threadContexts[cpu_id]->activate();
        break;
      default:
        panic("Invalid type to generate ipi\n");
    }
}

bool
Iob::receiveJBusInterrupt(int cpu_id, int source, uint64_t d0, uint64_t d1)
{
    // If we are already dealing with an interrupt for that cpu we can't deal
    // with another one right now... come back later
    if (jIntBusy[cpu_id].busy)
        return false;

    DPRINTF(Iob, "Receiving jBus interrupt: %d for cpu %d vec %d\n",
            source, cpu_id, jIntVec);

    jIntBusy[cpu_id].busy = true;
    jIntBusy[cpu_id].source = source;
    jBusData0[cpu_id] = d0;
    jBusData1[cpu_id] = d1;

    ic->post(cpu_id, SparcISA::IT_INT_VEC, jIntVec);
    return true;
}

void
Iob::addressRanges(AddrRangeList &range_list)
{
    range_list.clear();
    range_list.push_back(RangeSize(iobManAddr, iobManSize));
    range_list.push_back(RangeSize(iobJBusAddr, iobJBusSize));
}


void
Iob::serialize(std::ostream &os)
{

    SERIALIZE_SCALAR(jIntVec);
    SERIALIZE_ARRAY(jBusData0, MaxNiagaraProcs);
    SERIALIZE_ARRAY(jBusData1, MaxNiagaraProcs);
    for (int x = 0; x < NumDeviceIds; x++) {
        nameOut(os, csprintf("%s.Int%d", name(), x));
        paramOut(os, "cpu", intMan[x].cpu);
        paramOut(os, "vector", intMan[x].vector);
        paramOut(os, "mask", intCtl[x].mask);
        paramOut(os, "pend", intCtl[x].pend);
    };
    for (int x = 0; x < MaxNiagaraProcs; x++) {
        nameOut(os, csprintf("%s.jIntBusy%d", name(), x));
        paramOut(os, "busy", jIntBusy[x].busy);
        paramOut(os, "source", jIntBusy[x].source);
    };
}

void
Iob::unserialize(Checkpoint *cp, const std::string &section)
{
    UNSERIALIZE_SCALAR(jIntVec);
    UNSERIALIZE_ARRAY(jBusData0, MaxNiagaraProcs);
    UNSERIALIZE_ARRAY(jBusData1, MaxNiagaraProcs);
    for (int x = 0; x < NumDeviceIds; x++) {
        paramIn(cp, csprintf("%s.Int%d", name(), x), "cpu", intMan[x].cpu);
        paramIn(cp, csprintf("%s.Int%d", name(), x), "vector", intMan[x].vector);
        paramIn(cp, csprintf("%s.Int%d", name(), x), "mask", intCtl[x].mask);
        paramIn(cp, csprintf("%s.Int%d", name(), x), "pend", intCtl[x].pend);
    };
    for (int x = 0; x < MaxNiagaraProcs; x++) {
        paramIn(cp, csprintf("%s.jIntBusy%d", name(), x), "busy", jIntBusy[x].busy);
        paramIn(cp, csprintf("%s.jIntBusy%d", name(), x), "source", jIntBusy[x].source);
    };
}

Iob *
IobParams::create()
{
    return new Iob(this);
}
