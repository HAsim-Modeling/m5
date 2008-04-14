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
 * Authors: Ali G. Saidi
 *          Nathan L. Binkert
 */

#include "base/misc.hh"
#include "dev/platform.hh"
#include "sim/sim_exit.hh"

using namespace std;
using namespace TheISA;

Platform::Platform(const Params *p)
    : SimObject(p), intrctrl(p->intrctrl)
{
}

Platform::~Platform()
{
}

void
Platform::postPciInt(int line)
{
   panic("No PCI interrupt support in platform.");
}

void
Platform::clearPciInt(int line)
{
   panic("No PCI interrupt support in platform.");
}

Addr
Platform::pciToDma(Addr pciAddr) const
{
   panic("No PCI dma support in platform.");
   M5_DUMMY_RETURN
}

void
Platform::registerPciDevice(uint8_t bus, uint8_t dev, uint8_t func, uint8_t intr)
{
    uint32_t bdf = bus << 16 | dev << 8 | func << 0;
    if (pciDevices.find(bdf) != pciDevices.end())
        fatal("Two PCI devices have same bus:device:function\n");

    if (intLines.test(intr))
        fatal("Two PCI devices have same interrupt line: %d\n", intr);

    pciDevices.insert(bdf);

    intLines.set(intr);
}
