/*
 * Copyright (c) 2007 MIPS Technologies, Inc.  All Rights Reserved
 *
 * This software is part of the M5 simulator.
 *
 * THIS IS A LEGAL AGREEMENT.  BY DOWNLOADING, USING, COPYING, CREATING
 * DERIVATIVE WORKS, AND/OR DISTRIBUTING THIS SOFTWARE YOU ARE AGREEING
 * TO THESE TERMS AND CONDITIONS.
 *
 * Permission is granted to use, copy, create derivative works and
 * distribute this software and such derivative works for any purpose,
 * so long as (1) the copyright notice above, this grant of permission,
 * and the disclaimer below appear in all copies and derivative works
 * made, (2) the copyright notice above is augmented as appropriate to
 * reflect the addition of any new copyrightable work in a derivative
 * work (e.g., Copyright (c) <Publication Year> Copyright Owner), and (3)
 * the name of MIPS Technologies, Inc. (¡ÈMIPS¡É) is not used in any
 * advertising or publicity pertaining to the use or distribution of
 * this software without specific, written prior authorization.
 *
 * THIS SOFTWARE IS PROVIDED ¡ÈAS IS.¡É  MIPS MAKES NO WARRANTIES AND
 * DISCLAIMS ALL WARRANTIES, WHETHER EXPRESS, STATUTORY, IMPLIED OR
 * OTHERWISE, INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND
 * NON-INFRINGEMENT OF THIRD PARTY RIGHTS, REGARDING THIS SOFTWARE.
 * IN NO EVENT SHALL MIPS BE LIABLE FOR ANY DAMAGES, INCLUDING DIRECT,
 * INDIRECT, INCIDENTAL, CONSEQUENTIAL, SPECIAL, OR PUNITIVE DAMAGES OF
 * ANY KIND OR NATURE, ARISING OUT OF OR IN CONNECTION WITH THIS AGREEMENT,
 * THIS SOFTWARE AND/OR THE USE OF THIS SOFTWARE, WHETHER SUCH LIABILITY
 * IS ASSERTED ON THE BASIS OF CONTRACT, TORT (INCLUDING NEGLIGENCE OR
 * STRICT LIABILITY), OR OTHERWISE, EVEN IF MIPS HAS BEEN WARNED OF THE
 * POSSIBILITY OF ANY SUCH LOSS OR DAMAGE IN ADVANCE.
 *
 * Authors: Richard Strong 
 *
 */

/**
 * @file
 * Declaration of top level class for the Malta chipset. This class just
 * retains pointers to all its children so the children can communicate.
 */

#ifndef __DEV_MALTA_HH__
#define __DEV_MALTA_HH__

#include "dev/platform.hh"
#include "params/Malta.hh"

class IdeController;
class MaltaCChip;
class MaltaPChip;
class MaltaIO;
class System;

/**
  * Top level class for Malta Chipset emulation.
  * This structure just contains pointers to all the
  * children so the children can commnicate to do the
  * read work
  */

class Malta : public Platform
{
  public:
    /** Max number of CPUs in a Malta */
    static const int Max_CPUs = 64;

    /** Pointer to the system */
    System *system;

    /** Pointer to the MaltaIO device which has the RTC */
    MaltaIO *io;

    /** Pointer to the Malta CChip.
     * The chip contains some configuration information and
     * all the interrupt mask and status registers
     */
    MaltaCChip *cchip;

    /** Pointer to the Malta PChip.
     * The pchip is the interface to the PCI bus, in our case
     * it does not have to do much.
     */
    MaltaPChip *pchip;

    int intr_sum_type[Malta::Max_CPUs];
    int ipi_pending[Malta::Max_CPUs];

  public:
    /**
     * Constructor for the Malta Class.
     * @param name name of the object
     * @param s system the object belongs to
     * @param intctrl pointer to the interrupt controller
     */
    typedef MaltaParams Params;
    Malta(const Params *p);

    /**
     * Return the interrupting frequency to MipsAccess
     * @return frequency of RTC interrupts
     */
    virtual Tick intrFrequency();

    /**
     * Cause the cpu to post a serial interrupt to the CPU.
     */
    virtual void postConsoleInt();

    /**
     * Clear a posted CPU interrupt (id=55)
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

    /**
     * Serialize this object to the given output stream.
     * @param os The stream to serialize to.
     */
    virtual void serialize(std::ostream &os);

    /**
     * Reconstruct the state of this object from a checkpoint.
     * @param cp The checkpoint use.
     * @param section The section name of this object
     */
    virtual void unserialize(Checkpoint *cp, const std::string &section);
};

#endif // __DEV_MALTA_HH__
