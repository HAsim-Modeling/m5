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
 * Authors: Gabe Black
 */

#ifndef __ARCH_ALPHA_INTERRUPT_HH__
#define __ARCH_ALPHA_INTERRUPT_HH__

#include "arch/alpha/faults.hh"
#include "arch/alpha/isa_traits.hh"
#include "base/compiler.hh"
#include "cpu/thread_context.hh"

namespace AlphaISA
{
    class Interrupts
    {
      protected:
        uint64_t interrupts[NumInterruptLevels];
        uint64_t intstatus;

      public:
        Interrupts()
        {
            memset(interrupts, 0, sizeof(interrupts));
            intstatus = 0;
            newInfoSet = false;
        }

        void post(int int_num, int index)
        {
            DPRINTF(Interrupt, "Interrupt %d:%d posted\n", int_num, index);

            if (int_num < 0 || int_num >= NumInterruptLevels)
                panic("int_num out of bounds\n");

            if (index < 0 || index >= sizeof(uint64_t) * 8)
                panic("int_num out of bounds\n");

            interrupts[int_num] |= 1 << index;
            intstatus |= (ULL(1) << int_num);
        }

        void clear(int int_num, int index)
        {
            DPRINTF(Interrupt, "Interrupt %d:%d cleared\n", int_num, index);

            if (int_num < 0 || int_num >= TheISA::NumInterruptLevels)
                panic("int_num out of bounds\n");

            if (index < 0 || index >= sizeof(uint64_t) * 8)
                panic("int_num out of bounds\n");

            interrupts[int_num] &= ~(1 << index);
            if (interrupts[int_num] == 0)
                intstatus &= ~(ULL(1) << int_num);
        }

        void clear_all()
        {
            DPRINTF(Interrupt, "Interrupts all cleared\n");

            memset(interrupts, 0, sizeof(interrupts));
            intstatus = 0;
        }

        void serialize(std::ostream &os)
        {
            SERIALIZE_ARRAY(interrupts, NumInterruptLevels);
            SERIALIZE_SCALAR(intstatus);
        }

        void unserialize(Checkpoint *cp, const std::string &section)
        {
            UNSERIALIZE_ARRAY(interrupts, NumInterruptLevels);
            UNSERIALIZE_SCALAR(intstatus);
        }

        bool check_interrupts(ThreadContext * tc) const
        {
            return (intstatus != 0) && !(tc->readPC() & 0x3);
        }

        Fault getInterrupt(ThreadContext * tc)
        {
            int ipl = 0;
            int summary = 0;

            if (tc->readMiscRegNoEffect(IPR_ASTRR))
                panic("asynchronous traps not implemented\n");

            if (tc->readMiscRegNoEffect(IPR_SIRR)) {
                for (int i = INTLEVEL_SOFTWARE_MIN;
                     i < INTLEVEL_SOFTWARE_MAX; i++) {
                    if (tc->readMiscRegNoEffect(IPR_SIRR) & (ULL(1) << i)) {
                        // See table 4-19 of 21164 hardware reference
                        ipl = (i - INTLEVEL_SOFTWARE_MIN) + 1;
                        summary |= (ULL(1) << i);
                    }
                }
            }

            uint64_t interrupts = intstatus;
            if (interrupts) {
                for (int i = INTLEVEL_EXTERNAL_MIN;
                    i < INTLEVEL_EXTERNAL_MAX; i++) {
                    if (interrupts & (ULL(1) << i)) {
                        // See table 4-19 of 21164 hardware reference
                        ipl = i;
                        summary |= (ULL(1) << i);
                    }
                }
            }

            if (ipl && ipl > tc->readMiscRegNoEffect(IPR_IPLR)) {
                newIpl = ipl;
                newSummary = summary;
                newInfoSet = true;
                DPRINTF(Flow, "Interrupt! IPLR=%d ipl=%d summary=%x\n",
                        tc->readMiscRegNoEffect(IPR_IPLR), ipl, summary);

                return new InterruptFault;
            } else {
                return NoFault;
            }
        }

        void updateIntrInfo(ThreadContext *tc)
        {
            assert(newInfoSet);
            tc->setMiscRegNoEffect(IPR_ISR, newSummary);
            tc->setMiscRegNoEffect(IPR_INTID, newIpl);
            newInfoSet = false;
        }

        uint64_t get_vec(int int_num)
        {
            panic("Shouldn't be called for Alpha\n");
            M5_DUMMY_RETURN
        }

      private:
        bool newInfoSet;
        int newIpl;
        int newSummary;
    };
}

#endif

