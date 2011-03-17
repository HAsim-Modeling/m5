/*
 * Copyright (c) 2003, 2004
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
 */

#ifndef __SPARC_LINUX_PROCESS_HH__
#define __SPARC_LINUX_PROCESS_HH__

#include "arch/sparc/linux/linux.hh"
#include "arch/sparc/syscallreturn.hh"
#include "arch/sparc/process.hh"
#include "sim/process.hh"

namespace SparcISA {

//This contains all of the common elements of a SPARC Linux process which
//are not shared by other operating systems. The rest come from the common
//SPARC process class.
class SparcLinuxProcess
{
  public:
    SparcLinuxProcess();

     /// Array of syscall descriptors, indexed by call number.
    static SyscallDesc syscallDescs[];

     /// Array of 32 bit compatibility syscall descriptors,
     /// indexed by call number.
    static SyscallDesc syscall32Descs[];

    SyscallDesc* getDesc(int callnum);
    SyscallDesc* getDesc32(int callnum);

    const int Num_Syscall_Descs;
    const int Num_Syscall32_Descs;
};

/// A process with emulated SPARC/Linux syscalls.
class Sparc32LinuxProcess : public SparcLinuxProcess, public Sparc32LiveProcess
{
  public:
    /// Constructor.
    Sparc32LinuxProcess(LiveProcessParams * params, ObjectFile *objFile);

    SyscallDesc* getDesc(int callnum)
    {
        return SparcLinuxProcess::getDesc32(callnum);
    }

    void handleTrap(int trapNum, ThreadContext *tc);
};

/// A process with emulated 32 bit SPARC/Linux syscalls.
class Sparc64LinuxProcess : public SparcLinuxProcess, public Sparc64LiveProcess
{
  public:
    /// Constructor.
    Sparc64LinuxProcess(LiveProcessParams * params, ObjectFile *objFile);

    SyscallDesc* getDesc(int callnum)
    {
        return SparcLinuxProcess::getDesc(callnum);
    }

    void handleTrap(int trapNum, ThreadContext *tc);
};

SyscallReturn getresuidFunc(SyscallDesc *desc, int num,
                                 LiveProcess *p, ThreadContext *tc);

} // namespace SparcISA
#endif // __SPARC_LINUX_PROCESS_HH__
