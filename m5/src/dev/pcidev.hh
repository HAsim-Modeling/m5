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
 *          Andrew L. Schultz
 *          Nathan L. Binkert
 *
 */

/* @file
 * Interface for devices using PCI configuration
 */

#ifndef __DEV_PCIDEV_HH__
#define __DEV_PCIDEV_HH__

#include <cstring>

#include "dev/io_device.hh"
#include "dev/pcireg.h"
#include "dev/platform.hh"
#include "params/PciDevice.hh"
#include "sim/byteswap.hh"

#define BAR_IO_MASK 0x3
#define BAR_MEM_MASK 0xF
#define BAR_IO_SPACE_BIT 0x1
#define BAR_IO_SPACE(x) ((x) & BAR_IO_SPACE_BIT)
#define BAR_NUMBER(x) (((x) - PCI0_BASE_ADDR0) >> 0x2);



/**
 * PCI device, base implementation is only config space.
 */
class PciDev : public DmaDevice
{
    class PciConfigPort : public SimpleTimingPort
    {
      protected:
        PciDev *device;

        virtual Tick recvAtomic(PacketPtr pkt);

        virtual void getDeviceAddressRanges(AddrRangeList &resp,
                                            bool &snoop);

        Platform *platform;

        int busId;
        int deviceId;
        int functionId;

        Addr configAddr;

      public:
        PciConfigPort(PciDev *dev, int busid, int devid, int funcid,
                      Platform *p);
    };

  public:
    typedef PciDeviceParams Params;
    const Params *
    params() const
    {
        return dynamic_cast<const Params *>(_params);
    }

  protected:
    /** The current config space.  */
    PCIConfig config;

    /** The size of the BARs */
    uint32_t BARSize[6];

    /** The current address mapping of the BARs */
    Addr BARAddrs[6];

    /**
     * Does the given address lie within the space mapped by the given
     * base address register?
     */
    bool
    isBAR(Addr addr, int bar) const
    {
        assert(bar >= 0 && bar < 6);
        return BARAddrs[bar] <= addr && addr < BARAddrs[bar] + BARSize[bar];
    }

    /**
     * Which base address register (if any) maps the given address?
     * @return The BAR number (0-5 inclusive), or -1 if none.
     */
    int
    getBAR(Addr addr)
    {
        for (int i = 0; i <= 5; ++i)
            if (isBAR(addr, i))
                return i;

        return -1;
    }

    /**
     * Which base address register (if any) maps the given address?
     * @param addr The address to check.
     * @retval bar The BAR number (0-5 inclusive),
     *             only valid if return value is true.
     * @retval offs The offset from the base address,
     *              only valid if return value is true.
     * @return True iff address maps to a base address register's region.
     */
    bool
    getBAR(Addr addr, int &bar, Addr &offs)
    {
        int b = getBAR(addr);
        if (b < 0)
            return false;

        offs = addr - BARAddrs[b];
        bar = b;
        return true;
    }

  protected:
    Platform *plat;
    Tick pioDelay;
    Tick configDelay;
    PciConfigPort *configPort;

    /**
     * Write to the PCI config space data that is stored locally. This may be
     * overridden by the device but at some point it will eventually call this
     * for normal operations that it does not need to override.
     * @param pkt packet containing the write the offset into config space
     */
    virtual Tick writeConfig(PacketPtr pkt);


    /**
     * Read from the PCI config space data that is stored locally. This may be
     * overridden by the device but at some point it will eventually call this
     * for normal operations that it does not need to override.
     * @param pkt packet containing the write the offset into config space
     */
    virtual Tick readConfig(PacketPtr pkt);

  public:
    Addr pciToDma(Addr pciAddr) const
    { return plat->pciToDma(pciAddr); }

    void
    intrPost()
    { plat->postPciInt(letoh(config.interruptLine)); }

    void
    intrClear()
    { plat->clearPciInt(letoh(config.interruptLine)); }

    uint8_t
    interruptLine()
    { return letoh(config.interruptLine); }

    /** return the address ranges that this device responds to.
     * @params range_list range list to populate with ranges
     */
    void addressRanges(AddrRangeList &range_list);

    /**
     * Constructor for PCI Dev. This function copies data from the
     * config file object PCIConfigData and registers the device with
     * a PciConfigAll object.
     */
    PciDev(const Params *params);

    virtual void init();

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


    virtual unsigned int drain(Event *de);

    virtual Port *getPort(const std::string &if_name, int idx = -1)
    {
        if (if_name == "config") {
            if (configPort != NULL)
                panic("pciconfig port already connected to.");
            configPort = new PciConfigPort(this, params()->pci_bus,
                    params()->pci_dev, params()->pci_func,
                    params()->platform);
            return configPort;
        }
        return DmaDevice::getPort(if_name, idx);
    }

};
#endif // __DEV_PCIDEV_HH__
