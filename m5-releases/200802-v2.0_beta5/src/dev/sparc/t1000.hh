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
 */

/**
 * @file
 * Declaration of top level class for the T1000 platform chips. This class just
 * retains pointers to all its children so the children can communicate.
 */

#ifndef __DEV_T1000_HH__
#define __DEV_T1000_HH__

#include "dev/platform.hh"
#include "params/T1000.hh"

class IdeController;
class System;

class T1000 : public Platform
{
  public:
    /** Pointer to the system */
    System *system;

  public:
    typedef T1000Params Params;
    /**
     * Constructor for the Tsunami Class.
     * @param name name of the object
     * @param s system the object belongs to
     * @param intctrl pointer to the interrupt controller
     */
    T1000(const Params *p);

    /**
     * Return the interrupting frequency to AlphaAccess
     * @return frequency of RTC interrupts
     */
    virtual Tick intrFrequency();

    /**
     * Cause the cpu to post a serial interrupt to the CPU.
     */
    virtual void postConsoleInt();

    /**
     * Clear a posted CPU interrupt
     */
    virtual void clearConsoleInt();

    /**
     * Cause the chipset to post a cpi interrupt to the CPU.
     */
    virtual void postPciInt(int line);

    /**
     * Clear a posted PCI->CPU interrupt
     */
    virtual void clearPciInt(int line);


    virtual Addr pciToDma(Addr pciAddr) const;

    /**
     * Calculate the configuration address given a bus/dev/func.
     */
    virtual Addr calcConfigAddr(int bus, int dev, int func);
};

#endif // __DEV_T1000_HH__
