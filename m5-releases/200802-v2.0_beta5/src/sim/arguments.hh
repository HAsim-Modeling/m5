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

#ifndef __SIM_ARGUMENTS_HH__
#define __SIM_ARGUMENTS_HH__

#include <assert.h>

#include "arch/vtophys.hh"
#include "base/refcnt.hh"
#include "mem/vport.hh"
#include "sim/host.hh"

class ThreadContext;

class Arguments
{
  protected:
    ThreadContext *tc;
    int number;
    uint64_t getArg(bool fp = false);

  protected:
    class Data : public RefCounted
    {
      public:
        Data(){}
        ~Data();

      private:
        std::list<char *> data;

      public:
        char *alloc(size_t size);
    };

    RefCountingPtr<Data> data;

  public:
    Arguments(ThreadContext *ctx, int n = 0)
        : tc(ctx), number(n), data(NULL)
        { assert(number >= 0); data = new Data;}
    Arguments(const Arguments &args)
        : tc(args.tc), number(args.number), data(args.data) {}
    ~Arguments() {}

    ThreadContext *getThreadContext() const { return tc; }

    const Arguments &operator=(const Arguments &args) {
        tc = args.tc;
        number = args.number;
        data = args.data;
        return *this;
    }

    // for checking if an argument is NULL
    bool operator!() {
        return getArg() == 0;
    }

    Arguments &operator++() {
        ++number;
        assert(number >= 0);
        return *this;
    }

    Arguments operator++(int) {
        Arguments args = *this;
        ++number;
        assert(number >= 0);
        return args;
    }

    Arguments &operator--() {
        --number;
        assert(number >= 0);
        return *this;
    }

    Arguments operator--(int) {
        Arguments args = *this;
        --number;
        assert(number >= 0);
        return args;
    }

    const Arguments &operator+=(int index) {
        number += index;
        assert(number >= 0);
        return *this;
    }

    const Arguments &operator-=(int index) {
        number -= index;
        assert(number >= 0);
        return *this;
    }

    Arguments operator[](int index) {
        return Arguments(tc, index);
    }

    template <class T>
    operator T() {
        assert(sizeof(T) <= sizeof(uint64_t));
        T data = static_cast<T>(getArg());
        return data;
    }

    template <class T>
    operator T *() {
        T *buf = (T *)data->alloc(sizeof(T));
        CopyData(tc, buf, getArg(), sizeof(T));
        return buf;
    }

    operator char *() {
        char *buf = data->alloc(2048);
        CopyStringOut(tc, buf, getArg(), 2048);
        return buf;
    }
};

#endif // __SIM_ARGUMENTS_HH__
