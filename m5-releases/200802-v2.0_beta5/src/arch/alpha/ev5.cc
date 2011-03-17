/*
 * Copyright (c) 2002, 2003, 2004, 2005
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
 * Authors: Steven K. Reinhardt
 *          Nathan L. Binkert
 */

#include "arch/alpha/faults.hh"
#include "arch/alpha/isa_traits.hh"
#include "arch/alpha/kernel_stats.hh"
#include "arch/alpha/osfpal.hh"
#include "arch/alpha/tlb.hh"
#include "arch/alpha/kgdb.h"
#include "base/remote_gdb.hh"
#include "base/stats/events.hh"
#include "config/full_system.hh"
#include "cpu/base.hh"
#include "cpu/simple_thread.hh"
#include "cpu/thread_context.hh"
#include "sim/debug.hh"
#include "sim/sim_exit.hh"

#if FULL_SYSTEM

using namespace EV5;

////////////////////////////////////////////////////////////////////////
//
//  Machine dependent functions
//
void
AlphaISA::initCPU(ThreadContext *tc, int cpuId)
{
    initIPRs(tc, cpuId);

    tc->setIntReg(16, cpuId);
    tc->setIntReg(0, cpuId);

    AlphaISA::AlphaFault *reset = new AlphaISA::ResetFault;

    tc->setPC(tc->readMiscRegNoEffect(IPR_PAL_BASE) + reset->vect());
    tc->setNextPC(tc->readPC() + sizeof(MachInst));

    delete reset;
}


template <class CPU>
void
AlphaISA::processInterrupts(CPU *cpu)
{
    //Check if there are any outstanding interrupts
    //Handle the interrupts
    int ipl = 0;
    int summary = 0;

    if (cpu->readMiscRegNoEffect(IPR_ASTRR))
        panic("asynchronous traps not implemented\n");

    if (cpu->readMiscRegNoEffect(IPR_SIRR)) {
        for (int i = INTLEVEL_SOFTWARE_MIN;
             i < INTLEVEL_SOFTWARE_MAX; i++) {
            if (cpu->readMiscRegNoEffect(IPR_SIRR) & (ULL(1) << i)) {
                // See table 4-19 of the 21164 hardware reference
                ipl = (i - INTLEVEL_SOFTWARE_MIN) + 1;
                summary |= (ULL(1) << i);
            }
        }
    }

    uint64_t interrupts = cpu->intr_status();

    if (interrupts) {
        for (int i = INTLEVEL_EXTERNAL_MIN;
             i < INTLEVEL_EXTERNAL_MAX; i++) {
            if (interrupts & (ULL(1) << i)) {
                // See table 4-19 of the 21164 hardware reference
                ipl = i;
                summary |= (ULL(1) << i);
            }
        }
    }

    if (ipl && ipl > cpu->readMiscRegNoEffect(IPR_IPLR)) {
        cpu->setMiscRegNoEffect(IPR_ISR, summary);
        cpu->setMiscRegNoEffect(IPR_INTID, ipl);
        cpu->trap(new InterruptFault);
        DPRINTF(Flow, "Interrupt! IPLR=%d ipl=%d summary=%x\n",
                cpu->readMiscRegNoEffect(IPR_IPLR), ipl, summary);
    }

}

template <class CPU>
void
AlphaISA::zeroRegisters(CPU *cpu)
{
    // Insure ISA semantics
    // (no longer very clean due to the change in setIntReg() in the
    // cpu model.  Consider changing later.)
    cpu->thread->setIntReg(ZeroReg, 0);
    cpu->thread->setFloatReg(ZeroReg, 0.0);
}

Fault
SimpleThread::hwrei()
{
    if (!(readPC() & 0x3))
        return new UnimplementedOpcodeFault;

    setNextPC(readMiscRegNoEffect(AlphaISA::IPR_EXC_ADDR));

    if (!misspeculating()) {
        if (kernelStats)
            kernelStats->hwrei();
    }

    // FIXME: XXX check for interrupts? XXX
    return NoFault;
}

int
AlphaISA::MiscRegFile::getInstAsid()
{
    return EV5::ITB_ASN_ASN(ipr[IPR_ITB_ASN]);
}

int
AlphaISA::MiscRegFile::getDataAsid()
{
    return EV5::DTB_ASN_ASN(ipr[IPR_DTB_ASN]);
}

#endif

////////////////////////////////////////////////////////////////////////
//
//
//
void
AlphaISA::initIPRs(ThreadContext *tc, int cpuId)
{
    for (int i = 0; i < NumInternalProcRegs; ++i) {
        tc->setMiscRegNoEffect(i, 0);
    }

    tc->setMiscRegNoEffect(IPR_PAL_BASE, EV5::PalBase);
    tc->setMiscRegNoEffect(IPR_MCSR, 0x6);
    tc->setMiscRegNoEffect(IPR_PALtemp16, cpuId);
}

AlphaISA::MiscReg
AlphaISA::MiscRegFile::readIpr(int idx, ThreadContext *tc)
{
    uint64_t retval = 0;	// return value, default 0

    switch (idx) {
      case AlphaISA::IPR_PALtemp0:
      case AlphaISA::IPR_PALtemp1:
      case AlphaISA::IPR_PALtemp2:
      case AlphaISA::IPR_PALtemp3:
      case AlphaISA::IPR_PALtemp4:
      case AlphaISA::IPR_PALtemp5:
      case AlphaISA::IPR_PALtemp6:
      case AlphaISA::IPR_PALtemp7:
      case AlphaISA::IPR_PALtemp8:
      case AlphaISA::IPR_PALtemp9:
      case AlphaISA::IPR_PALtemp10:
      case AlphaISA::IPR_PALtemp11:
      case AlphaISA::IPR_PALtemp12:
      case AlphaISA::IPR_PALtemp13:
      case AlphaISA::IPR_PALtemp14:
      case AlphaISA::IPR_PALtemp15:
      case AlphaISA::IPR_PALtemp16:
      case AlphaISA::IPR_PALtemp17:
      case AlphaISA::IPR_PALtemp18:
      case AlphaISA::IPR_PALtemp19:
      case AlphaISA::IPR_PALtemp20:
      case AlphaISA::IPR_PALtemp21:
      case AlphaISA::IPR_PALtemp22:
      case AlphaISA::IPR_PALtemp23:
      case AlphaISA::IPR_PAL_BASE:

      case AlphaISA::IPR_IVPTBR:
      case AlphaISA::IPR_DC_MODE:
      case AlphaISA::IPR_MAF_MODE:
      case AlphaISA::IPR_ISR:
      case AlphaISA::IPR_EXC_ADDR:
      case AlphaISA::IPR_IC_PERR_STAT:
      case AlphaISA::IPR_DC_PERR_STAT:
      case AlphaISA::IPR_MCSR:
      case AlphaISA::IPR_ASTRR:
      case AlphaISA::IPR_ASTER:
      case AlphaISA::IPR_SIRR:
      case AlphaISA::IPR_ICSR:
      case AlphaISA::IPR_ICM:
      case AlphaISA::IPR_DTB_CM:
      case AlphaISA::IPR_IPLR:
      case AlphaISA::IPR_INTID:
      case AlphaISA::IPR_PMCTR:
        // no side-effect
        retval = ipr[idx];
        break;

      case AlphaISA::IPR_CC:
        retval |= ipr[idx] & ULL(0xffffffff00000000);
        retval |= tc->getCpuPtr()->curCycle()  & ULL(0x00000000ffffffff);
        break;

      case AlphaISA::IPR_VA:
        retval = ipr[idx];
        break;

      case AlphaISA::IPR_VA_FORM:
      case AlphaISA::IPR_MM_STAT:
      case AlphaISA::IPR_IFAULT_VA_FORM:
      case AlphaISA::IPR_EXC_MASK:
      case AlphaISA::IPR_EXC_SUM:
        retval = ipr[idx];
        break;

      case AlphaISA::IPR_DTB_PTE:
        {
            AlphaISA::TlbEntry &entry
                = tc->getDTBPtr()->index(!tc->misspeculating());

            retval |= ((uint64_t)entry.ppn & ULL(0x7ffffff)) << 32;
            retval |= ((uint64_t)entry.xre & ULL(0xf)) << 8;
            retval |= ((uint64_t)entry.xwe & ULL(0xf)) << 12;
            retval |= ((uint64_t)entry.fonr & ULL(0x1)) << 1;
            retval |= ((uint64_t)entry.fonw & ULL(0x1))<< 2;
            retval |= ((uint64_t)entry.asma & ULL(0x1)) << 4;
            retval |= ((uint64_t)entry.asn & ULL(0x7f)) << 57;
        }
        break;

        // write only registers
      case AlphaISA::IPR_HWINT_CLR:
      case AlphaISA::IPR_SL_XMIT:
      case AlphaISA::IPR_DC_FLUSH:
      case AlphaISA::IPR_IC_FLUSH:
      case AlphaISA::IPR_ALT_MODE:
      case AlphaISA::IPR_DTB_IA:
      case AlphaISA::IPR_DTB_IAP:
      case AlphaISA::IPR_ITB_IA:
      case AlphaISA::IPR_ITB_IAP:
        panic("Tried to read write only register %d\n", idx);
        break;

      default:
        // invalid IPR
        panic("Tried to read from invalid ipr %d\n", idx);
        break;
    }

    return retval;
}

#ifdef DEBUG
// Cause the simulator to break when changing to the following IPL
int break_ipl = -1;
#endif

void
AlphaISA::MiscRegFile::setIpr(int idx, uint64_t val, ThreadContext *tc)
{
    uint64_t old;

    if (tc->misspeculating())
        return;

    switch (idx) {
      case AlphaISA::IPR_PALtemp0:
      case AlphaISA::IPR_PALtemp1:
      case AlphaISA::IPR_PALtemp2:
      case AlphaISA::IPR_PALtemp3:
      case AlphaISA::IPR_PALtemp4:
      case AlphaISA::IPR_PALtemp5:
      case AlphaISA::IPR_PALtemp6:
      case AlphaISA::IPR_PALtemp7:
      case AlphaISA::IPR_PALtemp8:
      case AlphaISA::IPR_PALtemp9:
      case AlphaISA::IPR_PALtemp10:
      case AlphaISA::IPR_PALtemp11:
      case AlphaISA::IPR_PALtemp12:
      case AlphaISA::IPR_PALtemp13:
      case AlphaISA::IPR_PALtemp14:
      case AlphaISA::IPR_PALtemp15:
      case AlphaISA::IPR_PALtemp16:
      case AlphaISA::IPR_PALtemp17:
      case AlphaISA::IPR_PALtemp18:
      case AlphaISA::IPR_PALtemp19:
      case AlphaISA::IPR_PALtemp20:
      case AlphaISA::IPR_PALtemp21:
      case AlphaISA::IPR_PALtemp22:
      case AlphaISA::IPR_PAL_BASE:
      case AlphaISA::IPR_IC_PERR_STAT:
      case AlphaISA::IPR_DC_PERR_STAT:
      case AlphaISA::IPR_PMCTR:
        // write entire quad w/ no side-effect
        ipr[idx] = val;
        break;

      case AlphaISA::IPR_CC_CTL:
        // This IPR resets the cycle counter.  We assume this only
        // happens once... let's verify that.
        assert(ipr[idx] == 0);
        ipr[idx] = 1;
        break;

      case AlphaISA::IPR_CC:
        // This IPR only writes the upper 64 bits.  It's ok to write
        // all 64 here since we mask out the lower 32 in rpcc (see
        // isa_desc).
        ipr[idx] = val;
        break;

      case AlphaISA::IPR_PALtemp23:
        // write entire quad w/ no side-effect
        old = ipr[idx];
        ipr[idx] = val;
#if FULL_SYSTEM
        if (tc->getKernelStats())
            tc->getKernelStats()->context(old, val, tc);
#endif
        break;

      case AlphaISA::IPR_DTB_PTE:
        // write entire quad w/ no side-effect, tag is forthcoming
        ipr[idx] = val;
        break;

      case AlphaISA::IPR_EXC_ADDR:
        // second least significant bit in PC is always zero
        ipr[idx] = val & ~2;
        break;

      case AlphaISA::IPR_ASTRR:
      case AlphaISA::IPR_ASTER:
        // only write least significant four bits - privilege mask
        ipr[idx] = val & 0xf;
        break;

      case AlphaISA::IPR_IPLR:
#ifdef DEBUG
        if (break_ipl != -1 && break_ipl == (val & 0x1f))
            debug_break();
#endif

        // only write least significant five bits - interrupt level
        ipr[idx] = val & 0x1f;
#if FULL_SYSTEM
        if (tc->getKernelStats())
            tc->getKernelStats()->swpipl(ipr[idx]);
#endif
        break;

      case AlphaISA::IPR_DTB_CM:
#if FULL_SYSTEM
        if (val & 0x18) {
            if (tc->getKernelStats())
                tc->getKernelStats()->mode(TheISA::Kernel::user, tc);
        } else {
            if (tc->getKernelStats())
                tc->getKernelStats()->mode(TheISA::Kernel::kernel, tc);
        }
#endif

      case AlphaISA::IPR_ICM:
        // only write two mode bits - processor mode
        ipr[idx] = val & 0x18;
        break;

      case AlphaISA::IPR_ALT_MODE:
        // only write two mode bits - processor mode
        ipr[idx] = val & 0x18;
        break;

      case AlphaISA::IPR_MCSR:
        // more here after optimization...
        ipr[idx] = val;
        break;

      case AlphaISA::IPR_SIRR:
        // only write software interrupt mask
        ipr[idx] = val & 0x7fff0;
        break;

      case AlphaISA::IPR_ICSR:
        ipr[idx] = val & ULL(0xffffff0300);
        break;

      case AlphaISA::IPR_IVPTBR:
      case AlphaISA::IPR_MVPTBR:
        ipr[idx] = val & ULL(0xffffffffc0000000);
        break;

      case AlphaISA::IPR_DC_TEST_CTL:
        ipr[idx] = val & 0x1ffb;
        break;

      case AlphaISA::IPR_DC_MODE:
      case AlphaISA::IPR_MAF_MODE:
        ipr[idx] = val & 0x3f;
        break;

      case AlphaISA::IPR_ITB_ASN:
        ipr[idx] = val & 0x7f0;
        break;

      case AlphaISA::IPR_DTB_ASN:
        ipr[idx] = val & ULL(0xfe00000000000000);
        break;

      case AlphaISA::IPR_EXC_SUM:
      case AlphaISA::IPR_EXC_MASK:
        // any write to this register clears it
        ipr[idx] = 0;
        break;

      case AlphaISA::IPR_INTID:
      case AlphaISA::IPR_SL_RCV:
      case AlphaISA::IPR_MM_STAT:
      case AlphaISA::IPR_ITB_PTE_TEMP:
      case AlphaISA::IPR_DTB_PTE_TEMP:
        // read-only registers
        panic("Tried to write read only ipr %d\n", idx);

      case AlphaISA::IPR_HWINT_CLR:
      case AlphaISA::IPR_SL_XMIT:
      case AlphaISA::IPR_DC_FLUSH:
      case AlphaISA::IPR_IC_FLUSH:
        // the following are write only
        ipr[idx] = val;
        break;

      case AlphaISA::IPR_DTB_IA:
        // really a control write
        ipr[idx] = 0;

        tc->getDTBPtr()->flushAll();
        break;

      case AlphaISA::IPR_DTB_IAP:
        // really a control write
        ipr[idx] = 0;

        tc->getDTBPtr()->flushProcesses();
        break;

      case AlphaISA::IPR_DTB_IS:
        // really a control write
        ipr[idx] = val;

        tc->getDTBPtr()->flushAddr(val,
                EV5::DTB_ASN_ASN(ipr[AlphaISA::IPR_DTB_ASN]));
        break;

      case AlphaISA::IPR_DTB_TAG: {
          struct AlphaISA::TlbEntry entry;

          // FIXME: granularity hints NYI...
          if (EV5::DTB_PTE_GH(ipr[AlphaISA::IPR_DTB_PTE]) != 0)
              panic("PTE GH field != 0");

          // write entire quad
          ipr[idx] = val;

          // construct PTE for new entry
          entry.ppn = EV5::DTB_PTE_PPN(ipr[AlphaISA::IPR_DTB_PTE]);
          entry.xre = EV5::DTB_PTE_XRE(ipr[AlphaISA::IPR_DTB_PTE]);
          entry.xwe = EV5::DTB_PTE_XWE(ipr[AlphaISA::IPR_DTB_PTE]);
          entry.fonr = EV5::DTB_PTE_FONR(ipr[AlphaISA::IPR_DTB_PTE]);
          entry.fonw = EV5::DTB_PTE_FONW(ipr[AlphaISA::IPR_DTB_PTE]);
          entry.asma = EV5::DTB_PTE_ASMA(ipr[AlphaISA::IPR_DTB_PTE]);
          entry.asn = EV5::DTB_ASN_ASN(ipr[AlphaISA::IPR_DTB_ASN]);

          // insert new TAG/PTE value into data TLB
          tc->getDTBPtr()->insert(val, entry);
      }
        break;

      case AlphaISA::IPR_ITB_PTE: {
          struct AlphaISA::TlbEntry entry;

          // FIXME: granularity hints NYI...
          if (EV5::ITB_PTE_GH(val) != 0)
              panic("PTE GH field != 0");

          // write entire quad
          ipr[idx] = val;

          // construct PTE for new entry
          entry.ppn = EV5::ITB_PTE_PPN(val);
          entry.xre = EV5::ITB_PTE_XRE(val);
          entry.xwe = 0;
          entry.fonr = EV5::ITB_PTE_FONR(val);
          entry.fonw = EV5::ITB_PTE_FONW(val);
          entry.asma = EV5::ITB_PTE_ASMA(val);
          entry.asn = EV5::ITB_ASN_ASN(ipr[AlphaISA::IPR_ITB_ASN]);

          // insert new TAG/PTE value into data TLB
          tc->getITBPtr()->insert(ipr[AlphaISA::IPR_ITB_TAG], entry);
      }
        break;

      case AlphaISA::IPR_ITB_IA:
        // really a control write
        ipr[idx] = 0;

        tc->getITBPtr()->flushAll();
        break;

      case AlphaISA::IPR_ITB_IAP:
        // really a control write
        ipr[idx] = 0;

        tc->getITBPtr()->flushProcesses();
        break;

      case AlphaISA::IPR_ITB_IS:
        // really a control write
        ipr[idx] = val;

        tc->getITBPtr()->flushAddr(val,
                EV5::ITB_ASN_ASN(ipr[AlphaISA::IPR_ITB_ASN]));
        break;

      default:
        // invalid IPR
        panic("Tried to write to invalid ipr %d\n", idx);
    }

    // no error...
}


void
AlphaISA::copyIprs(ThreadContext *src, ThreadContext *dest)
{
    for (int i = 0; i < NumInternalProcRegs; ++i) {
        dest->setMiscRegNoEffect(i, src->readMiscRegNoEffect(i));
    }
}

#if FULL_SYSTEM

/**
 * Check for special simulator handling of specific PAL calls.
 * If return value is false, actual PAL call will be suppressed.
 */
bool
SimpleThread::simPalCheck(int palFunc)
{
    if (kernelStats)
        kernelStats->callpal(palFunc, tc);

    switch (palFunc) {
      case PAL::halt:
        halt();
        if (--System::numSystemsRunning == 0)
            exitSimLoop("all cpus halted");
        break;

      case PAL::bpt:
      case PAL::bugchk:
        if (system->breakpoint())
            return false;
        break;
    }

    return true;
}

#endif // FULL_SYSTEM
