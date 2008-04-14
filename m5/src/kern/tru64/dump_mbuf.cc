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
 * Authors: Nathan L. Binkert
 */

#include <sys/types.h>
#include <algorithm>

#include "base/cprintf.hh"
#include "base/trace.hh"
#include "base/loader/symtab.hh"
#include "cpu/thread_context.hh"
#include "kern/tru64/mbuf.hh"
#include "sim/host.hh"
#include "sim/system.hh"
#include "sim/arguments.hh"
#include "arch/isa_traits.hh"
#include "arch/vtophys.hh"

using namespace TheISA;

namespace tru64 {

void
DumpMbuf(Arguments args)
{
    ThreadContext *tc = args.getThreadContext();
    StringWrap name(tc->getSystemPtr()->name());
    Addr addr = (Addr)args;
    struct mbuf m;

    CopyOut(tc, &m, addr, sizeof(m));

    int count = m.m_pkthdr.len;

    DPRINTFN("m=%#lx, m->m_pkthdr.len=%#d\n", addr, m.m_pkthdr.len);

    while (count > 0) {
        DPRINTFN("m=%#lx, m->m_data=%#lx, m->m_len=%d\n",
                 addr, m.m_data, m.m_len);
        char *buffer = new char[m.m_len];
        CopyOut(tc, buffer, m.m_data, m.m_len);
        DDUMPN((uint8_t *)buffer, m.m_len);
        delete [] buffer;

        count -= m.m_len;
        if (!m.m_next)
            break;

        CopyOut(tc, &m, m.m_next, sizeof(m));
    }
}

} // namespace Tru64
