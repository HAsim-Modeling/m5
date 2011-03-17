/*
 * Copyright (c) 2003, 2004, 2005, 2006
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

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <fstream>
#include <string>

#include "arch/vtophys.hh"
#include "base/annotate.hh"
#include "cpu/base.hh"
#include "cpu/thread_context.hh"
#include "cpu/quiesce_event.hh"
#include "arch/kernel_stats.hh"
#include "sim/pseudo_inst.hh"
#include "sim/serialize.hh"
#include "sim/sim_exit.hh"
#include "sim/stat_control.hh"
#include "sim/stats.hh"
#include "sim/system.hh"
#include "sim/debug.hh"
#include "sim/vptr.hh"

using namespace std;

using namespace Stats;
using namespace TheISA;

namespace PseudoInst
{
    void
    arm(ThreadContext *tc)
    {
        if (tc->getKernelStats())
            tc->getKernelStats()->arm();
    }

    void
    quiesce(ThreadContext *tc)
    {
        if (!tc->getCpuPtr()->params->do_quiesce)
            return;

        DPRINTF(Quiesce, "%s: quiesce()\n", tc->getCpuPtr()->name());

        tc->suspend();
        if (tc->getKernelStats())
            tc->getKernelStats()->quiesce();
    }

    void
    quiesceNs(ThreadContext *tc, uint64_t ns)
    {
        if (!tc->getCpuPtr()->params->do_quiesce || ns == 0)
            return;

        EndQuiesceEvent *quiesceEvent = tc->getQuiesceEvent();

        Tick resume = curTick + Clock::Int::ns * ns;

        quiesceEvent->reschedule(resume, true);

        DPRINTF(Quiesce, "%s: quiesceNs(%d) until %d\n",
                tc->getCpuPtr()->name(), ns, resume);

        tc->suspend();
        if (tc->getKernelStats())
            tc->getKernelStats()->quiesce();
    }

    void
    quiesceCycles(ThreadContext *tc, uint64_t cycles)
    {
        if (!tc->getCpuPtr()->params->do_quiesce || cycles == 0)
            return;

        EndQuiesceEvent *quiesceEvent = tc->getQuiesceEvent();

        Tick resume = curTick + tc->getCpuPtr()->ticks(cycles);

        quiesceEvent->reschedule(resume, true);

        DPRINTF(Quiesce, "%s: quiesceCycles(%d) until %d\n",
                tc->getCpuPtr()->name(), cycles, resume);

        tc->suspend();
        if (tc->getKernelStats())
            tc->getKernelStats()->quiesce();
    }

    uint64_t
    quiesceTime(ThreadContext *tc)
    {
        return (tc->readLastActivate() - tc->readLastSuspend()) / Clock::Int::ns;
    }

    void
    m5exit_old(ThreadContext *tc)
    {
        exitSimLoop("m5_exit_old instruction encountered");
    }

    void
    m5exit(ThreadContext *tc, Tick delay)
    {
        Tick when = curTick + delay * Clock::Int::ns;
        schedExitSimLoop("m5_exit instruction encountered", when);
    }

    void
    loadsymbol(ThreadContext *tc)
    {
        const string &filename = tc->getCpuPtr()->system->params()->symbolfile;
        if (filename.empty()) {
            return;
        }

        std::string buffer;
        ifstream file(filename.c_str());

        if (!file)
            fatal("file error: Can't open symbol table file %s\n", filename);

        while (!file.eof()) {
            getline(file, buffer);

            if (buffer.empty())
                continue;

            int idx = buffer.find(' ');
            if (idx == string::npos)
                continue;

            string address = "0x" + buffer.substr(0, idx);
            eat_white(address);
            if (address.empty())
                continue;

            // Skip over letter and space
            string symbol = buffer.substr(idx + 3);
            eat_white(symbol);
            if (symbol.empty())
                continue;

            Addr addr;
            if (!to_number(address, addr))
                continue;

            if (!tc->getSystemPtr()->kernelSymtab->insert(addr, symbol))
                continue;


            DPRINTF(Loader, "Loaded symbol: %s @ %#llx\n", symbol, addr);
        }
        file.close();
    }

    void
    resetstats(ThreadContext *tc, Tick delay, Tick period)
    {
        if (!tc->getCpuPtr()->params->do_statistics_insts)
            return;


        Tick when = curTick + delay * Clock::Int::ns;
        Tick repeat = period * Clock::Int::ns;

        Stats::StatEvent(false, true, when, repeat);
    }

    void
    dumpstats(ThreadContext *tc, Tick delay, Tick period)
    {
        if (!tc->getCpuPtr()->params->do_statistics_insts)
            return;


        Tick when = curTick + delay * Clock::Int::ns;
        Tick repeat = period * Clock::Int::ns;

        Stats::StatEvent(true, false, when, repeat);
    }

    void
    addsymbol(ThreadContext *tc, Addr addr, Addr symbolAddr)
    {
        char symb[100];
        CopyStringOut(tc, symb, symbolAddr, 100);
        std::string symbol(symb);

        DPRINTF(Loader, "Loaded symbol: %s @ %#llx\n", symbol, addr);

        tc->getSystemPtr()->kernelSymtab->insert(addr,symbol);
    }

    void
    anBegin(ThreadContext *tc, uint64_t cur)
    {
        Annotate::annotations.add(tc->getSystemPtr(), 0, cur >> 32, cur &
                0xFFFFFFFF, 0,0);
    }

    void
    anWait(ThreadContext *tc, uint64_t cur, uint64_t wait)
    {
        Annotate::annotations.add(tc->getSystemPtr(), 0, cur >> 32, cur &
                0xFFFFFFFF, wait >> 32, wait & 0xFFFFFFFF);
    }


    void
    dumpresetstats(ThreadContext *tc, Tick delay, Tick period)
    {
        if (!tc->getCpuPtr()->params->do_statistics_insts)
            return;


        Tick when = curTick + delay * Clock::Int::ns;
        Tick repeat = period * Clock::Int::ns;

        Stats::StatEvent(true, true, when, repeat);
    }

    void
    m5checkpoint(ThreadContext *tc, Tick delay, Tick period)
    {
        if (!tc->getCpuPtr()->params->do_checkpoint_insts)
            return;

        Tick when = curTick + delay * Clock::Int::ns;
        Tick repeat = period * Clock::Int::ns;

        schedExitSimLoop("checkpoint", when, repeat);
    }

    uint64_t
    readfile(ThreadContext *tc, Addr vaddr, uint64_t len, uint64_t offset)
    {
        const string &file = tc->getSystemPtr()->params()->readfile;
        if (file.empty()) {
            return ULL(0);
        }

        uint64_t result = 0;

        int fd = ::open(file.c_str(), O_RDONLY, 0);
        if (fd < 0)
            panic("could not open file %s\n", file);

        if (::lseek(fd, offset, SEEK_SET) < 0)
            panic("could not seek: %s", strerror(errno));

        char *buf = new char[len];
        char *p = buf;
        while (len > 0) {
            int bytes = ::read(fd, p, len);
            if (bytes <= 0)
                break;

            p += bytes;
            result += bytes;
            len -= bytes;
        }

        close(fd);
        CopyIn(tc, vaddr, buf, result);
        delete [] buf;
        return result;
    }

    void debugbreak(ThreadContext *tc)
    {
        debug_break();
    }

    void switchcpu(ThreadContext *tc)
    {
        exitSimLoop("switchcpu");
    }
}
