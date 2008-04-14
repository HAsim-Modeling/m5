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

#ifndef __CPU_PROFILE_HH__
#define __CPU_PROFILE_HH__

#include <map>

#include "arch/stacktrace.hh"
#include "cpu/static_inst.hh"
#include "sim/host.hh"

class ThreadContext;

class ProfileNode
{
  private:
    friend class FunctionProfile;

    typedef std::map<Addr, ProfileNode *> ChildList;
    ChildList children;

  public:
    Counter count;

  public:
    ProfileNode();

    void dump(const std::string &symbol, uint64_t id,
              const SymbolTable *symtab, std::ostream &os) const;
    void clear();
};

class Callback;
class FunctionProfile
{
  private:
    Callback *reset;
    const SymbolTable *symtab;
    ProfileNode top;
    std::map<Addr, Counter> pc_count;
    TheISA::StackTrace trace;

  public:
    FunctionProfile(const SymbolTable *symtab);
    ~FunctionProfile();

    ProfileNode *consume(ThreadContext *tc, StaticInstPtr inst);
    ProfileNode *consume(const std::vector<Addr> &stack);
    void clear();
    void dump(ThreadContext *tc, std::ostream &out) const;
    void sample(ProfileNode *node, Addr pc);
};

inline ProfileNode *
FunctionProfile::consume(ThreadContext *tc, StaticInstPtr inst)
{
    if (!trace.trace(tc, inst))
        return NULL;
    trace.dprintf();
    return consume(trace.getstack());
}

#endif // __CPU_PROFILE_HH__
