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
 * This device implements the niagara I/O Bridge chip. The device manages
 * internal (ipi) and external (serial, pci via jbus).
 */

#ifndef __DEV_SPARC_IOB_HH__
#define __DEV_SPARC_IOB_HH__

#include "base/range.hh"
#include "dev/io_device.hh"
#include "dev/disk_image.hh"
#include "params/Iob.hh"

class IntrControl;

const int  MaxNiagaraProcs = 32;
// IOB Managment Addresses
const Addr IntManAddr       = 0x0000;
const Addr IntManSize       = 0x0020;
const Addr IntCtlAddr       = 0x0400;
const Addr IntCtlSize       = 0x0020;
const Addr JIntVecAddr      = 0x0A00;
const Addr IntVecDisAddr    = 0x0800;
const Addr IntVecDisSize    = 0x0100;


// IOB Control Addresses
const Addr JIntData0Addr   = 0x0400;
const Addr JIntData1Addr   = 0x0500;
const Addr JIntDataA0Addr  = 0x0600;
const Addr JIntDataA1Addr  = 0x0700;
const Addr JIntBusyAddr    = 0x0900;
const Addr JIntBusySize    = 0x0100;
const Addr JIntABusyAddr   = 0x0B00;


// IOB Masks
const uint64_t IntManMask   = 0x01F3F;
const uint64_t IntCtlMask   = 0x00006;
const uint64_t JIntVecMask  = 0x0003F;
const uint64_t IntVecDis    = 0x31F3F;
const uint64_t JIntBusyMask = 0x0003F;


class Iob : public PioDevice
{
  private:
    IntrControl *ic;
    Addr iobManAddr;
    Addr iobManSize;
    Addr iobJBusAddr;
    Addr iobJBusSize;
    Tick pioDelay;

    enum DeviceId {
        Interal = 0,
        Error = 1,
        SSI = 2,
        Reserved = 3,
        NumDeviceIds
    };

    struct IntMan {
        int cpu;
        int vector;
    };

    struct IntCtl {
        bool mask;
        bool pend;
    };

    struct IntBusy {
        bool busy;
        int source;
    };

    enum Type {
        Interrupt,
        Reset,
        Idle,
        Resume
    };

    IntMan intMan[NumDeviceIds];
    IntCtl intCtl[NumDeviceIds];
    uint64_t jIntVec;
    uint64_t jBusData0[MaxNiagaraProcs];
    uint64_t jBusData1[MaxNiagaraProcs];
    IntBusy jIntBusy[MaxNiagaraProcs];

    void writeIob(PacketPtr pkt);
    void writeJBus(PacketPtr pkt);
    void readIob(PacketPtr pkt);
    void readJBus(PacketPtr pkt);

  public:
    typedef IobParams Params;
    Iob(const Params *p);

    const Params *
    params() const
    {
        return dynamic_cast<const Params *>(_params);
    }

    virtual Tick read(PacketPtr pkt);
    virtual Tick write(PacketPtr pkt);
    void generateIpi(Type type, int cpu_id, int vector);
    void receiveDeviceInterrupt(DeviceId devid);
    bool receiveJBusInterrupt(int cpu_id, int source, uint64_t d0,
                              uint64_t d1);

    void addressRanges(AddrRangeList &range_list);

    virtual void serialize(std::ostream &os);
    virtual void unserialize(Checkpoint *cp, const std::string &section);

};

#endif //__DEV_SPARC_IOB_HH__

