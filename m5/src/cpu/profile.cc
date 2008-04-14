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

#include <string>

#include "base/bitfield.hh"
#include "base/callback.hh"
#include "base/statistics.hh"
#include "base/trace.hh"
#include "base/loader/symtab.hh"
#include "cpu/base.hh"
#include "cpu/thread_context.hh"
#include "cpu/profile.hh"

using namespace std;

ProfileNode::ProfileNode()
    : count(0)
{ }

void
ProfileNode::dump(const string &symbol, uint64_t id, const SymbolTable *symtab,
                  ostream &os) const
{
    ccprintf(os, "%#x %s %d ", id, symbol, count);
    ChildList::const_iterator i, end = children.end();
    for (i = children.begin(); i != end; ++i) {
        const ProfileNode *node = i->second;
        ccprintf(os, "%#x ", (intptr_t)node);
    }

    ccprintf(os, "\n");

    for (i = children.begin(); i != end; ++i) {
        Addr addr = i->first;
        string symbol;
        if (addr == 1)
            symbol = "user";
        else if (addr == 2)
            symbol = "console";
        else if (addr == 3)
            symbol = "unknown";
        else if (!symtab->findSymbol(addr, symbol))
            panic("could not find symbol for address %#x\n", addr);

        const ProfileNode *node = i->second;
        node->dump(symbol, (intptr_t)node, symtab, os);
    }
}

void
ProfileNode::clear()
{
    count = 0;
    ChildList::iterator i, end = children.end();
    for (i = children.begin(); i != end; ++i)
        i->second->clear();
}

FunctionProfile::FunctionProfile(const SymbolTable *_symtab)
    : reset(0), symtab(_symtab)
{
    reset = new MakeCallback<FunctionProfile, &FunctionProfile::clear>(this);
    Stats::registerResetCallback(reset);
}

FunctionProfile::~FunctionProfile()
{
    if (reset)
        delete reset;
}

ProfileNode *
FunctionProfile::consume(const vector<Addr> &stack)
{
    ProfileNode *current = &top;
    for (int i = 0, size = stack.size(); i < size; ++i) {
        ProfileNode *&ptr = current->children[stack[size - i - 1]];
        if (ptr == NULL)
            ptr = new ProfileNode;

        current = ptr;
    }

    return current;
}

void
FunctionProfile::clear()
{
    top.clear();
    pc_count.clear();
}

void
FunctionProfile::dump(ThreadContext *tc, ostream &os) const
{
    ccprintf(os, ">>>PC data\n");
    map<Addr, Counter>::const_iterator i, end = pc_count.end();
    for (i = pc_count.begin(); i != end; ++i) {
        Addr pc = i->first;
        Counter count = i->second;

        std::string symbol;
        if (pc == 1)
            ccprintf(os, "user %d\n", count);
        else if (symtab->findSymbol(pc, symbol) && !symbol.empty())
            ccprintf(os, "%s %d\n", symbol, count);
        else
            ccprintf(os, "%#x %d\n", pc, count);
    }

    ccprintf(os, ">>>function data\n");
    top.dump("top", 0, symtab, os);
}

void
FunctionProfile::sample(ProfileNode *node, Addr pc)
{
    node->count++;

    Addr symaddr;
    if (symtab->findNearestAddr(pc, symaddr)) {
        pc_count[symaddr]++;
    } else {
        // record PC even if we don't have a symbol to avoid
        // silently biasing the histogram
        pc_count[pc]++;
    }
}
