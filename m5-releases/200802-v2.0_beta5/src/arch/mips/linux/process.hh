/*
 * Copyright (c) 2004
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
 *          Korey Sewell
 *
 */

#ifndef __MIPS_LINUX_PROCESS_HH__
#define __MIPS_LINUX_PROCESS_HH__

#include "arch/mips/process.hh"
#include "arch/mips/linux/linux.hh"
#include "sim/eventq.hh"

/// A process with emulated Mips/Linux syscalls.
class MipsLinuxProcess : public MipsLiveProcess
{
  public:
    /// Constructor.
    MipsLinuxProcess(LiveProcessParams * params, ObjectFile *objFile);

    void startup();

    virtual SyscallDesc* getDesc(int callnum);

    /// The target system's hostname.
    static const char *hostname;

    /// ID of the thread group leader for the process
    uint64_t __tgid;

    /// Array of syscall descriptors, indexed by call number.
    static SyscallDesc syscallDescs[];
    const int Num_Syscall_Descs;
};

#endif // __MIPS_LINUX_PROCESS_HH__
