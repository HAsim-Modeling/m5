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
 * Authors: Andrew L. Schultz
 *          Nathan L. Binkert
 */

/**
 * @file
 * Generic interface for platforms
 */

#ifndef __DEV_PLATFORM_HH__
#define __DEV_PLATFORM_HH__

#include <bitset>
#include <set>

#include "sim/sim_object.hh"
#include "arch/isa_traits.hh"
#include "params/Platform.hh"

class PciConfigAll;
class IntrControl;
class SimConsole;
class Uart;
class System;

class Platform : public SimObject
{
  public:
    /** Pointer to the interrupt controller */
    IntrControl *intrctrl;

    /** Pointer to the system for info about the memory system. */
    System *system;

  public:
    typedef PlatformParams Params;
    Platform(const Params *p);
    virtual ~Platform();
    virtual void postConsoleInt() = 0;
    virtual void clearConsoleInt() = 0;
    virtual Tick intrFrequency() = 0;
    virtual void postPciInt(int line);
    virtual void clearPciInt(int line);
    virtual Addr pciToDma(Addr pciAddr) const;
    virtual Addr calcConfigAddr(int bus, int dev, int func) = 0;
    virtual void registerPciDevice(uint8_t bus, uint8_t dev, uint8_t func,
            uint8_t intr);

  private:
    std::bitset<256> intLines;
    std::set<uint32_t> pciDevices;

};

#endif // __DEV_PLATFORM_HH__