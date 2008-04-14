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

#ifndef __ARCH_SPARC_TLB_MAP_HH__
#define __ARCH_SPARC_TLB_MAP_HH__

#include "arch/sparc/pagetable.hh"
#include <map>

namespace SparcISA
{

class TlbMap
{
  private:
    typedef std::map<TlbRange, TlbEntry*> RangeMap;
    RangeMap tree;

  public:
    typedef RangeMap::iterator iterator;

    iterator find(const TlbRange &r)
    {
        iterator i;

        i = tree.upper_bound(r);

        if (i == tree.begin())
            if (r.real == i->first.real &&
                r.partitionId == i->first.partitionId &&
                i->first.va < r.va + r.size &&
                i->first.va+i->first.size >= r.va &&
                (r.real || r.contextId == i->first.contextId))
                return i;
            else
                // Nothing could match, so return end()
                return tree.end();

        i--;

        if (r.real != i->first.real)
            return tree.end();
        if (!r.real && r.contextId != i->first.contextId)
            return tree.end();
        if (r.partitionId != i->first.partitionId)
            return tree.end();
        if (i->first.va <= r.va+r.size &&
            i->first.va+i->first.size >= r.va)
            return i;

        return tree.end();
    }

    bool intersect(const TlbRange &r)
    {
        iterator i;
        i = find(r);
        if (i != tree.end())
            return true;
        return false;
    }


    iterator insert(TlbRange &r, TlbEntry *d)
    {
        if (intersect(r))
            return tree.end();

        return tree.insert(std::make_pair<TlbRange,TlbEntry*>(r, d)).first;
    }

    size_t erase(TlbRange k)
    {
        return tree.erase(k);
    }

    void erase(iterator p)
    {
        tree.erase(p);
    }

    void erase(iterator p, iterator q)
    {
        tree.erase(p,q);
    }

    void clear()
    {
        tree.erase(tree.begin(), tree.end());
    }

    iterator begin()
    {
        return tree.begin();
    }

    iterator end()
    {
        return tree.end();
    }

    size_t size()
    {
        return tree.size();
    }

    bool empty()
    {
        return tree.empty();
    }

    void print()
    {
        iterator i;
        i = tree.begin();
        while (i != tree.end()) {
           std::cout << std::hex << i->first.va << " " << i->first.size << " " <<
                i->first.contextId << " " << i->first.partitionId << " " <<
                i->first.real << " " << i->second << std::endl;
            i++;
        }
    }

};

};

#endif // __ARCH_SPARC_TLB_MAP_HH__
