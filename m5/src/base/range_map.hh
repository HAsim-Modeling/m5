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

#ifndef __BASE_RANGE_MAP_HH__
#define __BASE_RANGE_MAP_HH__

#include "base/range.hh"

#include <map>

template <class T,class V>
class range_map
{
  private:
    typedef std::map<Range<T>,V> RangeMap;
    RangeMap tree;

  public:
    typedef typename RangeMap::iterator iterator;

    template <class U>
    const iterator find(const Range<U> &r)
    {
        iterator i;

        i = tree.upper_bound(r);

        if (i == tree.begin()) {
            if (i->first.start <= r.end && i->first.end >= r.start)
                return i;
            else
                // Nothing could match, so return end()
                return tree.end();
        }

        i--;

        if (i->first.start <= r.end && i->first.end >= r.start)
            return i;

        return tree.end();
    }

    template <class U>
    bool intersect(const Range<U> &r)
    {
        iterator i;
        i = find(r);
        if (i != tree.end())
            return true;
        return false;
    }


    template <class U,class W>
    iterator insert(const Range<U> &r, const W d)
    {
        if (intersect(r))
            return tree.end();

        return tree.insert(std::make_pair<Range<T>,V>(r, d)).first;
    }

    size_t erase(T k)
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
};


template <class T,class V>
class range_multimap
{
  private:
    typedef std::multimap<Range<T>,V> RangeMap;
    RangeMap tree;

  public:
    typedef typename RangeMap::iterator iterator;

    template <class U>
    std::pair<iterator,iterator> find(const Range<U> &r)
    {
        iterator i;
        iterator j;

        i = tree.lower_bound(r);

        if (i == tree.begin()) {
          if (i->first.start <= r.end && i->first.end >= r.start)
                return std::make_pair<iterator, iterator>(i,i);
          else
            // Nothing could match, so return end()
            return std::make_pair<iterator, iterator>(tree.end(), tree.end());
        }
        i--;

        if (i->first.start <= r.end && i->first.end >= r.start) {
            // we have at least one match
            j = i;

            i--;
            while (i->first.start <= r.end && i->first.end >=
                    r.start) {
                if (i == tree.begin())
                    break;
                i--;
            }
            if (i == tree.begin() && i->first.start <= r.end && i->first.end >=
                                        r.start)
                return std::make_pair<iterator, iterator>(i,j);
            i++;
            return std::make_pair<iterator, iterator>(i,j);

        }

        return std::make_pair<iterator, iterator>(tree.end(), tree.end());
    }

    template <class U>
    bool intersect(const Range<U> &r)
    {
        std::pair<iterator,iterator> p;
        p = find(r);
        if (p.first != tree.end())
            return true;
        return false;
    }


    template <class U,class W>
    iterator insert(const Range<U> &r, const W d)
    {
        std::pair<iterator,iterator> p;
        p = find(r);
        if (p.first->first.start == r.start && p.first->first.end == r.end ||
                p.first == tree.end())
            return tree.insert(std::make_pair<Range<T>,V>(r, d));
        else
            return tree.end();
    }

    size_t erase(T k)
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
};

#endif //__BASE_RANGE_MAP_HH__
