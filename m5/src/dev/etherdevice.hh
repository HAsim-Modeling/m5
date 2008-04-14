/*
 * Copyright (c) 2007
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
 * Base Ethernet Device declaration.
 */

#ifndef __DEV_ETHERDEVICE_HH__
#define __DEV_ETHERDEVICE_HH__

#include "dev/pcidev.hh"
#include "params/EtherDevice.hh"
#include "sim/sim_object.hh"

class EtherInt;

/**
 * The base EtherObject class, allows for an accesor function to a
 * simobj that returns the Port.
 */
class EtherDevice : public PciDev
{
  public:
    typedef EtherDeviceParams Params;
    EtherDevice(const Params *params)
        : PciDev(params)
    {}

    const Params *
    params() const
    {
        return dynamic_cast<const Params *>(_params);
    }

  public:
    /** Additional function to return the Port of a memory object. */
    virtual EtherInt *getEthPort(const std::string &if_name, int idx = -1) = 0;

};

#endif //__DEV_ETHERDEVICE_HH__
