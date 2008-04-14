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
 *          Ali G. Saidi
 */


#include "arch/sparc/isa_traits.hh"
#include "arch/sparc/linux/process.hh"
#include "arch/sparc/regfile.hh"

#include "base/trace.hh"
#include "cpu/thread_context.hh"
#include "kern/linux/linux.hh"

#include "sim/process.hh"
#include "sim/syscall_emul.hh"

using namespace std;
using namespace SparcISA;

SyscallDesc*
SparcLinuxProcess::getDesc(int callnum)
{
    if (callnum < 0 || callnum > Num_Syscall_Descs)
        return NULL;
    return &syscallDescs[callnum];
}

SyscallDesc*
SparcLinuxProcess::getDesc32(int callnum)
{
    if (callnum < 0 || callnum > Num_Syscall32_Descs)
        return NULL;
    return &syscall32Descs[callnum];
}

SparcLinuxProcess::SparcLinuxProcess() :
    Num_Syscall_Descs(284), //sizeof(syscallDescs) / sizeof(SyscallDesc)),
    Num_Syscall32_Descs(299) //sizeof(syscall32Descs) / sizeof(SyscallDesc))
{
    // The sparc syscall table must be <= 284 entries because that is all there
    // is space for.
    assert(Num_Syscall_Descs <= 284);
    // The sparc 32 bit syscall table bust be <= 299 entries because that is
    // all there is space for.
    assert(Num_Syscall_Descs <= 299);
}

Sparc32LinuxProcess::Sparc32LinuxProcess(LiveProcessParams * params,
                                         ObjectFile *objFile)
    : Sparc32LiveProcess(params, objFile)
{}

void Sparc32LinuxProcess::handleTrap(int trapNum, ThreadContext *tc)
{
    switch(trapNum)
    {
      case 0x10: //Linux 32 bit syscall trap
        tc->syscall(tc->readIntReg(1));
        break;
      default:
        SparcLiveProcess::handleTrap(trapNum, tc);
    }
}

Sparc64LinuxProcess::Sparc64LinuxProcess(LiveProcessParams * params,
                                         ObjectFile *objFile)
    : Sparc64LiveProcess(params, objFile)
{}

void Sparc64LinuxProcess::handleTrap(int trapNum, ThreadContext *tc)
{
    switch(trapNum)
    {
      //case 0x10: //Linux 32 bit syscall trap
      case 0x6d: //Linux 64 bit syscall trap
        tc->syscall(tc->readIntReg(1));
        break;
      default:
        SparcLiveProcess::handleTrap(trapNum, tc);
    }
}
