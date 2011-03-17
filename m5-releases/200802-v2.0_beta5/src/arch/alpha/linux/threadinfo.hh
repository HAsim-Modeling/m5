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
 * Authors: Ali G. Saidi
 *          Nathan L. Binkert
 */


#ifndef __ARCH_ALPHA_LINUX_LINUX_TREADNIFO_HH__
#define __ARCH_ALPHA_LINUX_LINUX_TREADNIFO_HH__

#include "cpu/thread_context.hh"
#include "sim/system.hh"
#include "sim/vptr.hh"

namespace Linux {

class ThreadInfo
{
  private:
    ThreadContext *tc;
    System *sys;
    Addr pcbb;

    template <typename T>
    bool
    get_data(const char *symbol, T &data)
    {
        Addr addr = 0;
        if (!sys->kernelSymtab->findAddress(symbol, addr))
            return false;

        CopyOut(tc, &data, addr, sizeof(T));

        data = TheISA::gtoh(data);

        return true;
    }

  public:
    ThreadInfo(ThreadContext *_tc, Addr _pcbb = 0)
        : tc(_tc), sys(tc->getSystemPtr()), pcbb(_pcbb)
    {

    }
    ~ThreadInfo()
    {}

    inline Addr
    curThreadInfo()
    {
        Addr addr = pcbb;
        Addr sp;

        if (!addr)
            addr = tc->readMiscRegNoEffect(TheISA::IPR_PALtemp23);

        FunctionalPort *p = tc->getPhysPort();
        p->readBlob(addr, (uint8_t *)&sp, sizeof(Addr));

        return sp & ~ULL(0x3fff);
    }

    inline Addr
    curTaskInfo(Addr thread_info = 0)
    {
        int32_t offset;
        if (!get_data("thread_info_task", offset))
            return 0;

        if (!thread_info)
            thread_info = curThreadInfo();

        Addr addr;
        CopyOut(tc, &addr, thread_info + offset, sizeof(addr));

        return addr;
    }

    int32_t
    curTaskPID(Addr thread_info = 0)
    {
        Addr offset;
        if (!get_data("task_struct_pid", offset))
            return -1;

        int32_t pid;
        CopyOut(tc, &pid, curTaskInfo(thread_info) + offset, sizeof(pid));

        return pid;
    }

    int64_t
    curTaskStart(Addr thread_info = 0)
    {
        Addr offset;
        if (!get_data("task_struct_start_time", offset))
            return -1;

        int64_t data;
        // start_time is actually of type timespec, but if we just
        // grab the first long, we'll get the seconds out of it
        CopyOut(tc, &data, curTaskInfo(thread_info) + offset, sizeof(data));

        return data;
    }

    std::string
    curTaskName(Addr thread_info = 0)
    {
        int32_t offset;
        int32_t size;

        if (!get_data("task_struct_comm", offset))
            return "FailureIn_curTaskName";

        if (!get_data("task_struct_comm_size", size))
            return "FailureIn_curTaskName";

        char buffer[size + 1];
        CopyStringOut(tc, buffer, curTaskInfo(thread_info) + offset, size);

        return buffer;
    }
};

/* namespace Linux */ }

#endif // __ARCH_ALPHA_LINUX_LINUX_THREADINFO_HH__
