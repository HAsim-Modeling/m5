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

#include "arch/alpha/regfile.hh"
#include "cpu/thread_context.hh"

namespace AlphaISA
{
    void
    RegFile::serialize(std::ostream &os)
    {
        intRegFile.serialize(os);
        floatRegFile.serialize(os);
        miscRegFile.serialize(os);
        SERIALIZE_SCALAR(pc);
        SERIALIZE_SCALAR(npc);
#if FULL_SYSTEM
        SERIALIZE_SCALAR(intrflag);
#endif
    }

    void
    RegFile::unserialize(Checkpoint *cp, const std::string &section)
    {
        intRegFile.unserialize(cp, section);
        floatRegFile.unserialize(cp, section);
        miscRegFile.unserialize(cp, section);
        UNSERIALIZE_SCALAR(pc);
        UNSERIALIZE_SCALAR(npc);
#if FULL_SYSTEM
        UNSERIALIZE_SCALAR(intrflag);
#endif
    }

    void
    copyRegs(ThreadContext *src, ThreadContext *dest)
    {
        // First loop through the integer registers.
        for (int i = 0; i < NumIntRegs; ++i) {
            dest->setIntReg(i, src->readIntReg(i));
        }

        // Then loop through the floating point registers.
        for (int i = 0; i < TheISA::NumFloatRegs; ++i) {
            dest->setFloatRegBits(i, src->readFloatRegBits(i));
        }

        // Copy misc. registers
        copyMiscRegs(src, dest);

        // Lastly copy PC/NPC
        dest->setPC(src->readPC());
        dest->setNextPC(src->readNextPC());
    }

    void
    copyMiscRegs(ThreadContext *src, ThreadContext *dest)
    {
        dest->setMiscRegNoEffect(AlphaISA::MISCREG_FPCR,
                src->readMiscRegNoEffect(AlphaISA::MISCREG_FPCR));
        dest->setMiscRegNoEffect(AlphaISA::MISCREG_UNIQ,
                src->readMiscRegNoEffect(AlphaISA::MISCREG_UNIQ));
        dest->setMiscRegNoEffect(AlphaISA::MISCREG_LOCKFLAG,
                src->readMiscRegNoEffect(AlphaISA::MISCREG_LOCKFLAG));
        dest->setMiscRegNoEffect(AlphaISA::MISCREG_LOCKADDR,
                src->readMiscRegNoEffect(AlphaISA::MISCREG_LOCKADDR));

        copyIprs(src, dest);
    }
}
