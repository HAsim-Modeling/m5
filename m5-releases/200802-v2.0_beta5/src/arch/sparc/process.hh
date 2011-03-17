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

#ifndef __SPARC_PROCESS_HH__
#define __SPARC_PROCESS_HH__

#include <string>
#include <vector>
#include "sim/byteswap.hh"
#include "sim/process.hh"

class ObjectFile;
class System;

class SparcLiveProcess : public LiveProcess
{
  protected:

    const Addr StackBias;

    //The locations of the fill and spill handlers
    Addr fillStart, spillStart;

    SparcLiveProcess(LiveProcessParams * params,
            ObjectFile *objFile, Addr _StackBias);

    void startup();

    template<class IntType>
    void argsInit(int pageSize);

  public:

    //Handles traps which request services from the operating system
    virtual void handleTrap(int trapNum, ThreadContext *tc);

    Addr readFillStart()
    { return fillStart; }

    Addr readSpillStart()
    { return spillStart; }

    virtual void flushWindows(ThreadContext *tc) = 0;
};

template<class IntType>
struct M5_auxv_t
{
    IntType a_type;
    union {
        IntType a_val;
        IntType a_ptr;
        IntType a_fcn;
    };

    M5_auxv_t()
    {}

    M5_auxv_t(IntType type, IntType val)
    {
        a_type = SparcISA::htog(type);
        a_val = SparcISA::htog(val);
    }
};

class Sparc32LiveProcess : public SparcLiveProcess
{
  protected:

    Sparc32LiveProcess(LiveProcessParams * params, ObjectFile *objFile) :
            SparcLiveProcess(params, objFile, 0)
    {
        // Set up stack. On SPARC Linux, stack goes from the top of memory
        // downward, less the hole for the kernel address space.
        stack_base = (Addr)0xf0000000ULL;

        // Set up region for mmaps.
        mmap_start = mmap_end = 0x70000000;
    }

    void startup();

  public:

    void argsInit(int intSize, int pageSize);

    void flushWindows(ThreadContext *tc);
};

class Sparc64LiveProcess : public SparcLiveProcess
{
  protected:

    Sparc64LiveProcess(LiveProcessParams * params, ObjectFile *objFile) :
            SparcLiveProcess(params, objFile, 2047)
    {
        // Set up stack. On SPARC Linux, stack goes from the top of memory
        // downward, less the hole for the kernel address space.
        stack_base = (Addr)0x80000000000ULL;

        // Set up region for mmaps.  Tru64 seems to start just above 0 and
        // grow up from there.
        mmap_start = mmap_end = 0xfffff80000000000ULL;
    }

    void startup();

  public:

    void argsInit(int intSize, int pageSize);

    void flushWindows(ThreadContext *tc);
};

#endif // __SPARC_PROCESS_HH__
