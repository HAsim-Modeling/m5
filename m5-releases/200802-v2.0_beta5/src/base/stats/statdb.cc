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

#include "base/misc.hh"
#include "base/trace.hh"
#include "base/statistics.hh"
#include "base/stats/statdb.hh"

using namespace std;

namespace Stats {
namespace Database {

StatData *
find(void *stat)
{
    stat_map_t::const_iterator i = map().find(stat);

    if (i == map().end())
        return NULL;

    return (*i).second;
}

void
regStat(void *stat, StatData *data)
{
    if (map().find(stat) != map().end())
        panic("shouldn't register stat twice!");

    stats().push_back(data);

#ifndef NDEBUG
    pair<stat_map_t::iterator, bool> result =
#endif
        map().insert(make_pair(stat, data));
    assert(result.second && "this should never fail");
    assert(map().find(stat) != map().end());
}

void
regPrint(void *stat)
{
    StatData *data = find(stat);
    assert(data);
    data->flags |= print;
}

TheDatabase &db()
{
    static TheDatabase db;
    return db;
}

/* namespace Database */ }
/* namespace Stats */ }
