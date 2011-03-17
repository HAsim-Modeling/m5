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
 *          Korey L. Sewell
 */
#include "arch/mips/types.hh"
#include "cpu/o3/thread_context.hh"

template <class Impl>
class MipsTC : public O3ThreadContext<Impl>
{
  public:
    virtual uint64_t readNextNPC()
    {
        return this->cpu->readNextNPC(this->thread->readTid());
    }

    virtual void setNextNPC(uint64_t val)
    {
        this->cpu->setNextNPC(val, this->thread->readTid());
    }

    virtual void changeRegFileContext(TheISA::RegContextParam param,
                                      TheISA::RegContextVal val)
    { panic("Not supported on Mips!"); }

    /** This function exits the thread context in the CPU and returns
     * 1 if the CPU has no more active threads (meaning it's OK to exit);
     * Used in syscall-emulation mode when a thread executes the 'exit'
     * syscall.
     */
    virtual int exit()
    {
        this->deallocate();

        // If there are still threads executing in the system
        if (this->cpu->numActiveThreads())
            return 0; // don't exit simulation
        else
            return 1; // exit simulation
    }
};
