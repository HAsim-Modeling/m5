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
 * Authors: Ali G. Saidi
 */

/**
 * @file
 * Port object definitions.
 */

#include "base/chunk_generator.hh"
#include "cpu/thread_context.hh"
#include "mem/vport.hh"

void
VirtualPort::readBlob(Addr addr, uint8_t *p, int size)
{
    Addr paddr;
    for (ChunkGenerator gen(addr, size, TheISA::PageBytes); !gen.done();
            gen.next())
    {
        if (tc)
            paddr = TheISA::vtophys(tc,gen.addr());
        else
            paddr = TheISA::vtophys(gen.addr());

        FunctionalPort::readBlob(paddr, p, gen.size());
        p += gen.size();
    }
}

void
VirtualPort::writeBlob(Addr addr, uint8_t *p, int size)
{
    Addr paddr;
    for (ChunkGenerator gen(addr, size, TheISA::PageBytes); !gen.done();
            gen.next())
    {
        if (tc)
            paddr = TheISA::vtophys(tc,gen.addr());
        else
            paddr = TheISA::vtophys(gen.addr());

        FunctionalPort::writeBlob(paddr, p, gen.size());
        p += gen.size();
    }
}

void
CopyOut(ThreadContext *tc, void *dest, Addr src, size_t cplen)
{
    uint8_t *dst = (uint8_t *)dest;
    VirtualPort *vp = tc->getVirtPort(tc);

    vp->readBlob(src, dst, cplen);

    tc->delVirtPort(vp);

}

void
CopyIn(ThreadContext *tc, Addr dest, void *source, size_t cplen)
{
    uint8_t *src = (uint8_t *)source;
    VirtualPort *vp = tc->getVirtPort(tc);

    vp->writeBlob(dest, src, cplen);

    tc->delVirtPort(vp);
}

void
CopyStringOut(ThreadContext *tc, char *dst, Addr vaddr, size_t maxlen)
{
    int len = 0;
    char *start = dst;
    VirtualPort *vp = tc->getVirtPort(tc);

    do {
        vp->readBlob(vaddr++, (uint8_t*)dst++, 1);
    } while (len < maxlen && start[len++] != 0 );

    tc->delVirtPort(vp);
    dst[len] = 0;
}

void
CopyStringIn(ThreadContext *tc, char *src, Addr vaddr)
{
    VirtualPort *vp = tc->getVirtPort(tc);
    for (ChunkGenerator gen(vaddr, strlen(src), TheISA::PageBytes); !gen.done();
            gen.next())
    {
        vp->writeBlob(gen.addr(), (uint8_t*)src, gen.size());
        src += gen.size();
    }
    tc->delVirtPort(vp);
}
