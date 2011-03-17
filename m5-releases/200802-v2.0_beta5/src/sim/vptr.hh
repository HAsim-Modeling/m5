/*
 * Copyright (c) 2004, 2005
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

#ifndef __ARCH_ALPHA_VPTR_HH__
#define __ARCH_ALPHA_VPTR_HH__

#include "arch/vtophys.hh"
#include "arch/isa_traits.hh"
#include "mem/vport.hh"

class ThreadContext;

template <class T>
class VPtr
{
  public:
    typedef T Type;

  protected:
    ThreadContext *tc;
    Addr ptr;
    Addr buffer[(sizeof(T)-1)/sizeof(Addr) + 1];

  public:
    explicit VPtr(ThreadContext *_tc, Addr p = 0)
        : tc(_tc), ptr(p)
    {
        refresh();
    }

    template <class U>
    VPtr(const VPtr<U> &vp)
        : tc(vp.tc), ptr(vp.ptr)
    {
        refresh();
    }

    ~VPtr()
    {}

    void
    refresh()
    {
        if (!ptr)
            return;

        VirtualPort *port = tc->getVirtPort(tc);
        port->readBlob(ptr, buffer, sizeof(T));
        tc->delVirtPort(port);
    }

    bool
    operator!() const
    {
        return ptr == 0;
    }

    VPtr<T>
    operator+(int offset)
    {
        return VPtr<T>(tc, ptr + offset);
    }

    const VPtr<T> &
    operator+=(int offset)
    {
        ptr += offset;
        refresh();

        return *this;
    }

    const VPtr<T> &
    operator=(Addr p)
    {
        ptr = p;
        refresh();

        return *this;
    }

    template <class U>
    const VPtr<T> &
    operator=(const VPtr<U> &vp)
    {
        tc = vp.tc;
        ptr = vp.ptr;
        refresh();

        return *this;
    }

    operator T *()
    {
        return (T *)buffer;
    }

    T *
    operator->()
    {
        return (T *)buffer;
    }

    T &
    operator*()
    {
        return *(T *)buffer;
    }
};

#endif // __ARCH_ALPHA_VPTR_HH__
