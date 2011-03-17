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
 * Authors: Nathan L. Binkert
 */

#ifndef __ARCH_SPARC_REMOTE_GDB_HH__
#define __ARCH_SPARC_REMOTE_GDB_HH__

#include <map>

#include "arch/sparc/types.hh"
#include "base/remote_gdb.hh"
#include "cpu/pc_event.hh"
#include "base/pollevent.hh"

class System;
class ThreadContext;
class PhysicalMemory;

namespace SparcISA
{
    class RemoteGDB : public BaseRemoteGDB
    {
      protected:
        enum RegisterConstants
        {
            RegG0 = 0, RegO0 = 8, RegL0 = 16, RegI0 = 24,
            RegF0 = 32,
            RegPc = 64, RegNpc, RegState, RegFsr, RegFprs, RegY,
            /*RegState contains data in same format as tstate */
            Reg32Y = 64, Reg32Psr = 65, Reg32Tbr = 66, Reg32Pc = 67,
            Reg32Npc = 68, Reg32Fsr = 69, Reg32Csr = 70,
            NumGDBRegs
        };

      public:
        RemoteGDB(System *system, ThreadContext *context);

        bool acc(Addr addr, size_t len);

      protected:
        void getregs();
        void setregs();

        void clearSingleStep();
        void setSingleStep();

        Addr nextBkpt;
    };
}

#endif /* __ARCH_SPARC_REMOTE_GDB_H__ */
