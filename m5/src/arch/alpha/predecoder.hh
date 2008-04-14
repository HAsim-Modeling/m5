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
 * Authors: Gabe Black
 */

#ifndef __ARCH_ALPHA_PREDECODER_HH__
#define __ARCH_ALPHA_PREDECODER_HH__

#include "arch/alpha/types.hh"
#include "base/misc.hh"
#include "config/full_system.hh"
#include "sim/host.hh"

class ThreadContext;

namespace AlphaISA
{
    class Predecoder
    {
      protected:
        ThreadContext * tc;
        //The extended machine instruction being generated
        ExtMachInst ext_inst;

      public:
        Predecoder(ThreadContext * _tc) : tc(_tc)
        {}

        ThreadContext * getTC()
        {
            return tc;
        }

        void setTC(ThreadContext * _tc)
        {
            tc = _tc;
        }

        void process()
        {
        }

        void reset()
        {}

        //Use this to give data to the predecoder. This should be used
        //when there is control flow.
        void moreBytes(Addr pc, Addr fetchPC, MachInst inst)
        {
            ext_inst = inst;
#if FULL_SYSTEM
	    ext_inst|=(static_cast<ExtMachInst>(pc & 0x1) << 32);
#endif
        }

        bool needMoreBytes()
        {
            return true;
        }

        bool extMachInstReady()
        {
            return true;
        }

        //This returns a constant reference to the ExtMachInst to avoid a copy
        const ExtMachInst & getExtMachInst()
        {
            return ext_inst;
        }
    };
};

#endif // __ARCH_ALPHA_PREDECODER_HH__
