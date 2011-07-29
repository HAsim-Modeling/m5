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
 * Authors: Nathan L. Binkert
 */

%module stats

%include "std_string.i"

%{
#include "base/statistics.hh"
#include "base/stats/mysql.hh"
#include "base/stats/text.hh"
#include "sim/stat_control.hh"
%}

namespace Stats {
void initSimStats();
void initText(const std::string &filename, bool desc=true, bool compat=true);
void initMySQL(std::string host, std::string database, std::string user,
    std::string passwd, std::string project, std::string name,
    std::string sample);

void StatEvent(bool dump, bool reset, Tick when = curTick, Tick repeat = 0);

void check();
void dump();
void reset();

/* namespace Stat */ }

%wrapper %{
// fix up module name to reflect the fact that it's inside the m5 package
#undef SWIG_name
#define SWIG_name "m5.internal._stats"
%}