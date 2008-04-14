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

#ifndef __BASE_STATS_STATDB_HH__
#define __BASE_STATS_STATDB_HH__

#include <iosfwd>
#include <list>
#include <map>
#include <string>

class Python;

namespace Stats {

class StatData;

namespace Database {

typedef std::map<void *, StatData *> stat_map_t;
typedef std::list<StatData *> stat_list_t;

// We wrap the database in a struct to make sure it is built in time.
struct TheDatabase
{
    stat_map_t map;
    stat_list_t stats;
};

TheDatabase &db();
inline stat_map_t &map() { return db().map; }
inline stat_list_t &stats() { return db().stats; }

StatData *find(void *stat);
void regStat(void *stat, StatData *data);
void regPrint(void *stat);

inline std::string name() { return "Statistics Database"; }

/* namespace Database */ }
/* namespace Stats */ }

#endif // __BASE_STATS_STATDB_HH__
