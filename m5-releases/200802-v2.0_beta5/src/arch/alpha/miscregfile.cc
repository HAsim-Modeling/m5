/*
 * Copyright (c) 2003, 2004, 2005
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
 *          Gabe M. Black
 *          Kevin T. Lim
 */

#include "arch/alpha/miscregfile.hh"
#include "base/misc.hh"

namespace AlphaISA
{

    void
    MiscRegFile::serialize(std::ostream &os)
    {
        SERIALIZE_SCALAR(fpcr);
        SERIALIZE_SCALAR(uniq);
        SERIALIZE_SCALAR(lock_flag);
        SERIALIZE_SCALAR(lock_addr);
        SERIALIZE_ARRAY(ipr, NumInternalProcRegs);
    }

    void
    MiscRegFile::unserialize(Checkpoint *cp, const std::string &section)
    {
        UNSERIALIZE_SCALAR(fpcr);
        UNSERIALIZE_SCALAR(uniq);
        UNSERIALIZE_SCALAR(lock_flag);
        UNSERIALIZE_SCALAR(lock_addr);
        UNSERIALIZE_ARRAY(ipr, NumInternalProcRegs);
    }

    MiscReg
    MiscRegFile::readRegNoEffect(int misc_reg)
    {
        switch(misc_reg) {
          case MISCREG_FPCR:
            return fpcr;
          case MISCREG_UNIQ:
            return uniq;
          case MISCREG_LOCKFLAG:
            return lock_flag;
          case MISCREG_LOCKADDR:
            return lock_addr;
          case MISCREG_INTR:
            return intr_flag;
          default:
            assert(misc_reg < NumInternalProcRegs);
            return ipr[misc_reg];
        }
    }

    MiscReg
    MiscRegFile::readReg(int misc_reg, ThreadContext *tc)
    {
        switch(misc_reg) {
          case MISCREG_FPCR:
            return fpcr;
          case MISCREG_UNIQ:
            return uniq;
          case MISCREG_LOCKFLAG:
            return lock_flag;
          case MISCREG_LOCKADDR:
            return lock_addr;
          case MISCREG_INTR:
            return intr_flag;
          default:
            return readIpr(misc_reg, tc);
        }
    }

    void
    MiscRegFile::setRegNoEffect(int misc_reg, const MiscReg &val)
    {
        switch(misc_reg) {
          case MISCREG_FPCR:
            fpcr = val;
            return;
          case MISCREG_UNIQ:
            uniq = val;
            return;
          case MISCREG_LOCKFLAG:
            lock_flag = val;
            return;
          case MISCREG_LOCKADDR:
            lock_addr = val;
            return;
          case MISCREG_INTR:
            intr_flag = val;
            return;
          default:
            assert(misc_reg < NumInternalProcRegs);
            ipr[misc_reg] = val;
            return;
        }
    }

    void
    MiscRegFile::setReg(int misc_reg, const MiscReg &val,
            ThreadContext *tc)
    {
        switch(misc_reg) {
          case MISCREG_FPCR:
            fpcr = val;
            return;
          case MISCREG_UNIQ:
            uniq = val;
            return;
          case MISCREG_LOCKFLAG:
            lock_flag = val;
            return;
          case MISCREG_LOCKADDR:
            lock_addr = val;
            return;
          case MISCREG_INTR:
            intr_flag = val;
            return;
          default:
            setIpr(misc_reg, val, tc);
            return;
        }
    }

}
