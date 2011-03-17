/*
 * Copyright (c) 2005
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
 * Defines a 8250 UART
 */

#ifndef __DEV_UART8250_HH__
#define __DEV_UART8250_HH__

#include "base/range.hh"
#include "dev/io_device.hh"
#include "dev/uart.hh"
#include "params/Uart8250.hh"

/* UART8250 Interrupt ID Register
 *  bit 0    Interrupt Pending 0 = true, 1 = false
 *  bit 2:1  ID of highest priority interrupt
 *  bit 7:3  zeroes
 */
const uint8_t IIR_NOPEND = 0x1;

// Interrupt IDs
const uint8_t IIR_MODEM = 0x00; /* Modem Status (lowest priority) */
const uint8_t IIR_TXID  = 0x02; /* Tx Data */
const uint8_t IIR_RXID  = 0x04; /* Rx Data */
const uint8_t IIR_LINE  = 0x06; /* Rx Line Status (highest priority)*/

const uint8_t UART_IER_RDI  = 0x01;
const uint8_t UART_IER_THRI = 0x02;
const uint8_t UART_IER_RLSI = 0x04;


const uint8_t UART_LSR_TEMT = 0x40;
const uint8_t UART_LSR_THRE = 0x20;
const uint8_t UART_LSR_DR   = 0x01;

const uint8_t UART_MCR_LOOP = 0x10;


class SimConsole;
class Platform;

class Uart8250 : public Uart
{
  protected:
    uint8_t IER, DLAB, LCR, MCR;
    Tick lastTxInt;

    class IntrEvent : public Event
    {
        protected:
            Uart8250 *uart;
            int intrBit;
        public:
            IntrEvent(Uart8250 *u, int bit);
            virtual void process();
            virtual const char *description() const;
            void scheduleIntr();
    };

    IntrEvent txIntrEvent;
    IntrEvent rxIntrEvent;

  public:
    typedef Uart8250Params Params;
    const Params *
    params() const
    {
        return dynamic_cast<const Params *>(_params);
    }
    Uart8250(const Params *p);

    virtual Tick read(PacketPtr pkt);
    virtual Tick write(PacketPtr pkt);
    virtual void addressRanges(AddrRangeList &range_list);

    /**
     * Inform the uart that there is data available.
     */
    virtual void dataAvailable();


    /**
     * Return if we have an interrupt pending
     * @return interrupt status
     */
    virtual bool intStatus() { return status ? true : false; }

    virtual void serialize(std::ostream &os);
    virtual void unserialize(Checkpoint *cp, const std::string &section);

};

#endif // __TSUNAMI_UART_HH__
