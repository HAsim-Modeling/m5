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
 * Authors: Gabe M. Black
 *          Ali G. Saidi
 */

#include "arch/alpha/isa_traits.hh"
#include "arch/alpha/process.hh"
#include "base/loader/object_file.hh"
#include "base/misc.hh"
#include "cpu/thread_context.hh"
#include "sim/system.hh"


using namespace AlphaISA;
using namespace std;

AlphaLiveProcess::AlphaLiveProcess(LiveProcessParams * params,
        ObjectFile *objFile)
    : LiveProcess(params, objFile)
{
    brk_point = objFile->dataBase() + objFile->dataSize() + objFile->bssSize();
    brk_point = roundUp(brk_point, VMPageSize);

    // Set up stack.  On Alpha, stack goes below text section.  This
    // code should get moved to some architecture-specific spot.
    stack_base = objFile->textBase() - (409600+4096);

    // Set up region for mmaps.  Tru64 seems to start just above 0 and
    // grow up from there.
    mmap_start = mmap_end = 0x10000;

    // Set pointer for next thread stack.  Reserve 8M for main stack.
    next_thread_stack_base = stack_base - (8 * 1024 * 1024);

}

void
AlphaLiveProcess::startup()
{
    if (checkpointRestored)
        return;

    argsInit(MachineBytes, VMPageSize);

    threadContexts[0]->setIntReg(GlobalPointerReg, objFile->globalPointer());
    //Opperate in user mode
    threadContexts[0]->setMiscRegNoEffect(IPR_ICM, 0x18);
    //No super page mapping
    threadContexts[0]->setMiscRegNoEffect(IPR_MCSR, 0);
    //Set this to 0 for now, but it should be unique for each process
    threadContexts[0]->setMiscRegNoEffect(IPR_DTB_ASN, M5_pid << 57);
}


