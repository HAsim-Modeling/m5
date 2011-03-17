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
 * Authors: Kevin T. Lim
 */

#include "base/misc.hh"
#include "base/trace.hh"
#include "cpu/thread_context.hh"

void
ThreadContext::compare(ThreadContext *one, ThreadContext *two)
{
    DPRINTF(Context, "Comparing thread contexts\n");

    // First loop through the integer registers.
    for (int i = 0; i < TheISA::NumIntRegs; ++i) {
        TheISA::IntReg t1 = one->readIntReg(i);
        TheISA::IntReg t2 = two->readIntReg(i);
        if (t1 != t2)
            panic("Int reg idx %d doesn't match, one: %#x, two: %#x",
                  i, t1, t2);
    }

    // Then loop through the floating point registers.
    for (int i = 0; i < TheISA::NumFloatRegs; ++i) {
        TheISA::FloatRegBits t1 = one->readFloatRegBits(i);
        TheISA::FloatRegBits t2 = two->readFloatRegBits(i);
        if (t1 != t2)
            panic("Float reg idx %d doesn't match, one: %#x, two: %#x",
                  i, t1, t2);
    }
#if FULL_SYSTEM
    for (int i = 0; i < TheISA::NumMiscRegs; ++i) {
        TheISA::MiscReg t1 = one->readMiscRegNoEffect(i);
        TheISA::MiscReg t2 = two->readMiscRegNoEffect(i);
        if (t1 != t2)
            panic("Misc reg idx %d doesn't match, one: %#x, two: %#x",
                  i, t1, t2);
    }
#endif

    Addr pc1 = one->readPC();
    Addr pc2 = two->readPC();
    if (pc1 != pc2)
        panic("PCs doesn't match, one: %#x, two: %#x", pc1, pc2);

    Addr npc1 = one->readNextPC();
    Addr npc2 = two->readNextPC();
    if (npc1 != npc2)
        panic("NPCs doesn't match, one: %#x, two: %#x", npc1, npc2);

    int id1 = one->readCpuId();
    int id2 = two->readCpuId();
    if (id1 != id2)
        panic("CPU ids don't match, one: %d, two: %d", id1, id2);
}
