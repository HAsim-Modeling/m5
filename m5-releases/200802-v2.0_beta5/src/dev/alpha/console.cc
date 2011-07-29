/*
 * Copyright (c) 2001, 2002, 2003, 2004, 2005
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
 * Authors: Nathan L. Binkert
 *          Ali G. Saidi
 *          Steven K. Reinhardt
 *          Erik G. Hallnor
 */

/** @file
 * Alpha Console Definition
 */

#include <cstddef>
#include <string>

#include "arch/alpha/system.hh"
#include "base/inifile.hh"
#include "base/str.hh"
#include "base/trace.hh"
#include "cpu/base.hh"
#include "cpu/thread_context.hh"
#include "dev/alpha/console.hh"
#include "dev/platform.hh"
#include "dev/simconsole.hh"
#include "dev/simple_disk.hh"
#include "mem/packet.hh"
#include "mem/packet_access.hh"
#include "mem/physical.hh"
#include "params/AlphaConsole.hh"
#include "sim/sim_object.hh"

using namespace std;
using namespace AlphaISA;

AlphaConsole::AlphaConsole(const Params *p)
    : BasicPioDevice(p), disk(p->disk), console(p->sim_console),
      system(p->system), cpu(p->cpu)
{

    pioSize = sizeof(struct AlphaAccess);

    alphaAccess = new Access();
    alphaAccess->last_offset = pioSize - 1;

    alphaAccess->version = ALPHA_ACCESS_VERSION;
    alphaAccess->diskUnit = 1;

    alphaAccess->diskCount = 0;
    alphaAccess->diskPAddr = 0;
    alphaAccess->diskBlock = 0;
    alphaAccess->diskOperation = 0;
    alphaAccess->outputChar = 0;
    alphaAccess->inputChar = 0;
    std::memset(alphaAccess->cpuStack, 0, sizeof(alphaAccess->cpuStack));

}

void
AlphaConsole::startup()
{
    system->setAlphaAccess(pioAddr);
    alphaAccess->numCPUs = system->getNumCPUs();
    alphaAccess->kernStart = system->getKernelStart();
    alphaAccess->kernEnd = system->getKernelEnd();
    alphaAccess->entryPoint = system->getKernelEntry();
    alphaAccess->mem_size = system->physmem->size();
    alphaAccess->cpuClock = cpu->frequency() / 1000000; // In MHz
    alphaAccess->intrClockFrequency = params()->platform->intrFrequency();
}

Tick
AlphaConsole::read(PacketPtr pkt)
{

    /** XXX Do we want to push the addr munging to a bus brige or something? So
     * the device has it's physical address and then the bridge adds on whatever
     * machine dependent address swizzle is required?
     */

    assert(pkt->getAddr() >= pioAddr && pkt->getAddr() < pioAddr + pioSize);

    Addr daddr = pkt->getAddr() - pioAddr;

    pkt->allocate();
    pkt->makeAtomicResponse();

    switch (pkt->getSize())
    {
        case sizeof(uint32_t):
            switch (daddr)
            {
                case offsetof(AlphaAccess, last_offset):
                    pkt->set(alphaAccess->last_offset);
                    break;
                case offsetof(AlphaAccess, version):
                    pkt->set(alphaAccess->version);
                    break;
                case offsetof(AlphaAccess, numCPUs):
                    pkt->set(alphaAccess->numCPUs);
                    break;
                case offsetof(AlphaAccess, intrClockFrequency):
                    pkt->set(alphaAccess->intrClockFrequency);
                    break;
                default:
                    /* Old console code read in everyting as a 32bit int
                     * we now break that for better error checking.
                     */
                  pkt->setBadAddress();
            }
            DPRINTF(AlphaConsole, "read: offset=%#x val=%#x\n", daddr,
                    pkt->get<uint32_t>());
            break;
        case sizeof(uint64_t):
            switch (daddr)
            {
                case offsetof(AlphaAccess, inputChar):
                    pkt->set(console->console_in());
                    break;
                case offsetof(AlphaAccess, cpuClock):
                    pkt->set(alphaAccess->cpuClock);
                    break;
                case offsetof(AlphaAccess, mem_size):
                    pkt->set(alphaAccess->mem_size);
                    break;
                case offsetof(AlphaAccess, kernStart):
                    pkt->set(alphaAccess->kernStart);
                    break;
                case offsetof(AlphaAccess, kernEnd):
                    pkt->set(alphaAccess->kernEnd);
                    break;
                case offsetof(AlphaAccess, entryPoint):
                    pkt->set(alphaAccess->entryPoint);
                    break;
                case offsetof(AlphaAccess, diskUnit):
                    pkt->set(alphaAccess->diskUnit);
                    break;
                case offsetof(AlphaAccess, diskCount):
                    pkt->set(alphaAccess->diskCount);
                    break;
                case offsetof(AlphaAccess, diskPAddr):
                    pkt->set(alphaAccess->diskPAddr);
                    break;
                case offsetof(AlphaAccess, diskBlock):
                    pkt->set(alphaAccess->diskBlock);
                    break;
                case offsetof(AlphaAccess, diskOperation):
                    pkt->set(alphaAccess->diskOperation);
                    break;
                case offsetof(AlphaAccess, outputChar):
                    pkt->set(alphaAccess->outputChar);
                    break;
                default:
                    int cpunum = (daddr - offsetof(AlphaAccess, cpuStack)) /
                                 sizeof(alphaAccess->cpuStack[0]);

                    if (cpunum >= 0 && cpunum < 64)
                        pkt->set(alphaAccess->cpuStack[cpunum]);
                    else
                        panic("Unknown 64bit access, %#x\n", daddr);
            }
            DPRINTF(AlphaConsole, "read: offset=%#x val=%#x\n", daddr,
                    pkt->get<uint64_t>());
            break;
        default:
          pkt->setBadAddress();
    }
    return pioDelay;
}

Tick
AlphaConsole::write(PacketPtr pkt)
{
    assert(pkt->getAddr() >= pioAddr && pkt->getAddr() < pioAddr + pioSize);
    Addr daddr = pkt->getAddr() - pioAddr;

    uint64_t val = pkt->get<uint64_t>();
    assert(pkt->getSize() == sizeof(uint64_t));

    switch (daddr) {
      case offsetof(AlphaAccess, diskUnit):
        alphaAccess->diskUnit = val;
        break;

      case offsetof(AlphaAccess, diskCount):
        alphaAccess->diskCount = val;
        break;

      case offsetof(AlphaAccess, diskPAddr):
        alphaAccess->diskPAddr = val;
        break;

      case offsetof(AlphaAccess, diskBlock):
        alphaAccess->diskBlock = val;
        break;

      case offsetof(AlphaAccess, diskOperation):
        if (val == 0x13)
            disk->read(alphaAccess->diskPAddr, alphaAccess->diskBlock,
                       alphaAccess->diskCount);
        else
            panic("Invalid disk operation!");

        break;

      case offsetof(AlphaAccess, outputChar):
        console->out((char)(val & 0xff));
        break;

      default:
        int cpunum = (daddr - offsetof(AlphaAccess, cpuStack)) /
                     sizeof(alphaAccess->cpuStack[0]);
        warn("%d: Trying to launch CPU number %d!", curTick, cpunum);
        assert(val > 0 && "Must not access primary cpu");
        if (cpunum >= 0 && cpunum < 64)
            alphaAccess->cpuStack[cpunum] = val;
        else
            panic("Unknown 64bit access, %#x\n", daddr);
    }

    pkt->makeAtomicResponse();

    return pioDelay;
}

void
AlphaConsole::Access::serialize(ostream &os)
{
    SERIALIZE_SCALAR(last_offset);
    SERIALIZE_SCALAR(version);
    SERIALIZE_SCALAR(numCPUs);
    SERIALIZE_SCALAR(mem_size);
    SERIALIZE_SCALAR(cpuClock);
    SERIALIZE_SCALAR(intrClockFrequency);
    SERIALIZE_SCALAR(kernStart);
    SERIALIZE_SCALAR(kernEnd);
    SERIALIZE_SCALAR(entryPoint);
    SERIALIZE_SCALAR(diskUnit);
    SERIALIZE_SCALAR(diskCount);
    SERIALIZE_SCALAR(diskPAddr);
    SERIALIZE_SCALAR(diskBlock);
    SERIALIZE_SCALAR(diskOperation);
    SERIALIZE_SCALAR(outputChar);
    SERIALIZE_SCALAR(inputChar);
    SERIALIZE_ARRAY(cpuStack,64);
}

void
AlphaConsole::Access::unserialize(Checkpoint *cp, const std::string &section)
{
    UNSERIALIZE_SCALAR(last_offset);
    UNSERIALIZE_SCALAR(version);
    UNSERIALIZE_SCALAR(numCPUs);
    UNSERIALIZE_SCALAR(mem_size);
    UNSERIALIZE_SCALAR(cpuClock);
    UNSERIALIZE_SCALAR(intrClockFrequency);
    UNSERIALIZE_SCALAR(kernStart);
    UNSERIALIZE_SCALAR(kernEnd);
    UNSERIALIZE_SCALAR(entryPoint);
    UNSERIALIZE_SCALAR(diskUnit);
    UNSERIALIZE_SCALAR(diskCount);
    UNSERIALIZE_SCALAR(diskPAddr);
    UNSERIALIZE_SCALAR(diskBlock);
    UNSERIALIZE_SCALAR(diskOperation);
    UNSERIALIZE_SCALAR(outputChar);
    UNSERIALIZE_SCALAR(inputChar);
    UNSERIALIZE_ARRAY(cpuStack, 64);
}

void
AlphaConsole::serialize(ostream &os)
{
    alphaAccess->serialize(os);
}

void
AlphaConsole::unserialize(Checkpoint *cp, const std::string &section)
{
    alphaAccess->unserialize(cp, section);
}

AlphaConsole *
AlphaConsoleParams::create()
{
    return new AlphaConsole(this);
}