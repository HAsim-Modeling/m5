/*
 * Copyright (c) 2005
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

#ifndef __ARCH_ALPHA_STACKTRACE_HH__
#define __ARCH_ALPHA_STACKTRACE_HH__

#include "base/trace.hh"
#include "cpu/static_inst.hh"

class ThreadContext;

namespace AlphaISA
{
    class StackTrace;

    class ProcessInfo
    {
      private:
        ThreadContext *tc;

        int thread_info_size;
        int task_struct_size;
        int task_off;
        int pid_off;
        int name_off;

      public:
        ProcessInfo(ThreadContext *_tc);

        Addr task(Addr ksp) const;
        int pid(Addr ksp) const;
        std::string name(Addr ksp) const;
    };

    class StackTrace
    {
      protected:
        typedef TheISA::MachInst MachInst;
      private:
        ThreadContext *tc;
        std::vector<Addr> stack;

      private:
        bool isEntry(Addr addr);
        bool decodePrologue(Addr sp, Addr callpc, Addr func, int &size, Addr &ra);
        bool decodeSave(MachInst inst, int &reg, int &disp);
        bool decodeStack(MachInst inst, int &disp);

        void trace(ThreadContext *tc, bool is_call);

      public:
        StackTrace();
        StackTrace(ThreadContext *tc, StaticInstPtr inst);
        ~StackTrace();

        void clear()
        {
            tc = 0;
            stack.clear();
        }

        bool valid() const { return tc != NULL; }
        bool trace(ThreadContext *tc, StaticInstPtr inst);

      public:
        const std::vector<Addr> &getstack() const { return stack; }

        static const int user = 1;
        static const int console = 2;
        static const int unknown = 3;

#if TRACING_ON
      private:
        void dump();

      public:
        void dprintf() { if (DTRACE(Stack)) dump(); }
#else
      public:
        void dprintf() {}
#endif
    };

    inline bool
    StackTrace::trace(ThreadContext *tc, StaticInstPtr inst)
    {
        if (!inst->isCall() && !inst->isReturn())
            return false;

        if (valid())
            clear();

        trace(tc, !inst->isReturn());
        return true;
    }
}

#endif // __ARCH_ALPHA_STACKTRACE_HH__
