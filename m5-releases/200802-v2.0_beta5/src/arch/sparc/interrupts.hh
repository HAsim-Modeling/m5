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
 *          Lisa R. Hsu
 */

#ifndef __ARCH_SPARC_INTERRUPT_HH__
#define __ARCH_SPARC_INTERRUPT_HH__

#include "arch/sparc/faults.hh"
#include "arch/sparc/isa_traits.hh"
#include "cpu/thread_context.hh"

namespace SparcISA
{

class Interrupts
{

  private:

    uint64_t interrupts[NumInterruptTypes];
    uint64_t intStatus;

  public:
    Interrupts()
    {
        clear_all();
    }

    int InterruptLevel(uint64_t softint)
    {
        if (softint & 0x10000 || softint & 0x1)
            return 14;

        int level = 15;
        while (level > 0 && !(1 << level & softint))
            level--;
        if (1 << level & softint)
            return level;
        return 0;
    }

    void post(int int_num, int index)
    {
        DPRINTF(Interrupt, "Interrupt %d:%d posted\n", int_num, index);
        assert(int_num >= 0 && int_num < NumInterruptTypes);
        assert(index >= 0 && index < 64);

        interrupts[int_num] |= ULL(1) << index;
        intStatus |= ULL(1) << int_num;
    }

    void clear(int int_num, int index)
    {
        DPRINTF(Interrupt, "Interrupt %d:%d cleared\n", int_num, index);
        assert(int_num >= 0 && int_num < NumInterruptTypes);
        assert(index >= 0 && index < 64);

        interrupts[int_num] &= ~(ULL(1) << index);
        if (!interrupts[int_num])
            intStatus &= ~(ULL(1) << int_num);
    }

    void clear_all()
    {
        for (int i = 0; i < NumInterruptTypes; ++i) {
            interrupts[i] = 0;
        }
        intStatus = 0;
    }

    bool check_interrupts(ThreadContext * tc) const
    {
        return intStatus;
    }

    Fault getInterrupt(ThreadContext * tc)
    {
        int hpstate = tc->readMiscRegNoEffect(MISCREG_HPSTATE);
        int pstate = tc->readMiscRegNoEffect(MISCREG_PSTATE);
        bool ie = pstate & PSTATE::ie;

        // THESE ARE IN ORDER OF PRIORITY
        // since there are early returns, and the highest
        // priority interrupts should get serviced,
        // it is v. important that new interrupts are inserted
        // in the right order of processing
        if (hpstate & HPSTATE::hpriv) {
            if (ie) {
                if (interrupts[IT_HINTP]) {
                    // This will be cleaned by a HINTP write
                    return new HstickMatch;
                }
                if (interrupts[IT_INT_VEC]) {
                    // this will be cleared by an ASI read (or write)
                    return new InterruptVector;
                }
            }
        } else {
            if (interrupts[IT_TRAP_LEVEL_ZERO]) {
                    // this is cleared by deasserting HPSTATE::tlz
                    return new TrapLevelZero;
            }
            // HStick matches always happen in priv mode (ie doesn't matter)
            if (interrupts[IT_HINTP]) {
                return new HstickMatch;
            }
            if (interrupts[IT_INT_VEC]) {
                // this will be cleared by an ASI read (or write)
                return new InterruptVector;
            }
            if (ie) {
                if (interrupts[IT_CPU_MONDO]) {
                    return new CpuMondo;
                }
                if (interrupts[IT_DEV_MONDO]) {
                    return new DevMondo;
                }
                if (interrupts[IT_SOFT_INT]) {
                    return new
                        InterruptLevelN(InterruptLevel(interrupts[IT_SOFT_INT]));
                }

                if (interrupts[IT_RES_ERROR]) {
                    return new ResumableError;
                }
            } // !hpriv && ie
        }  // !hpriv
        return NoFault;
    }

    void updateIntrInfo(ThreadContext * tc)
    {

    }

    uint64_t get_vec(int int_num)
    {
        assert(int_num >= 0 && int_num < NumInterruptTypes);
        return interrupts[int_num];
    }

    void serialize(std::ostream &os)
    {
        SERIALIZE_ARRAY(interrupts,NumInterruptTypes);
        SERIALIZE_SCALAR(intStatus);
    }

    void unserialize(Checkpoint *cp, const std::string &section)
    {
        UNSERIALIZE_ARRAY(interrupts,NumInterruptTypes);
        UNSERIALIZE_SCALAR(intStatus);
    }
};
} // namespace SPARC_ISA

#endif // __ARCH_SPARC_INTERRUPT_HH__