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
 *          Ali G. Saidi
 */

/*
 * @file
 * PCI Config space implementation.
 */

#ifndef __PCICONFIGALL_HH__
#define __PCICONFIGALL_HH__

#include "dev/pcireg.h"
#include "base/range.hh"
#include "dev/io_device.hh"
#include "params/PciConfigAll.hh"


/**
 * PCI Config Space
 * All of PCI config space needs to return -1 on Tsunami, except
 * the devices that exist. This device maps the entire bus config
 * space and passes the requests on to TsunamiPCIDev devices as
 * appropriate.
 */
class PciConfigAll : public PioDevice
{
  public:
    typedef PciConfigAllParams Params;
    const Params *params() const { return (const Params *)_params; }

    /**
     * Constructor for PCIConfigAll
     * @param p parameters structure
     */
    PciConfigAll(const Params *p);

    /**
     * Read something in PCI config space. If the device does not exist
     * -1 is returned, if the device does exist its PciDev::ReadConfig (or the
     * virtual function that overrides) it is called.
     * @param pkt Contains information about the read operation
     * @return Amount of time to do the read
     */
    virtual Tick read(PacketPtr pkt);

    /**
     * Write to PCI config spcae. If the device does not exit the simulator
     * panics. If it does it is passed on the PciDev::WriteConfig (or the virtual
     * function that overrides it).
     * @param pkt Contains information about the write operation
     * @return Amount of time to do the read
     */

    virtual Tick write(PacketPtr pkt);

    void addressRanges(AddrRangeList &range_list);

  private:
    Addr pioAddr;

};

#endif // __PCICONFIGALL_HH__
