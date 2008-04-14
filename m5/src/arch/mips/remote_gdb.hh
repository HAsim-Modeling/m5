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
 *
 * Authors: Nathan L. Binkert
 */

#ifndef __ARCH_MIPS_REMOTE_GDB_HH__
#define __ARCH_MIPS_REMOTE_GDB_HH__

#include "base/remote_gdb.hh"

namespace MipsISA
{
    class RemoteGDB : public BaseRemoteGDB
    {
      public:
        //These needs to be written to suit MIPS

        RemoteGDB(System *system, ThreadContext *context)
            : BaseRemoteGDB(system, context, 1)
        {}

        bool acc(Addr, size_t)
        { panic("acc not implemented for MIPS!"); }

        void getregs()
        { panic("getregs not implemented for MIPS!"); }

        void setregs()
        { panic("setregs not implemented for MIPS!"); }

        void clearSingleStep()
        { panic("clearSingleStep not implemented for MIPS!"); }

        void setSingleStep()
        { panic("setSingleStep not implemented for MIPS!"); }
    };
}

#endif /* __ARCH_ALPHA_REMOTE_GDB_H__ */
