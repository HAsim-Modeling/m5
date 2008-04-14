/*
 * Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006
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
 *          Steven K. Reinhardt
 */

#ifndef __BASE_TRACE_HH__
#define __BASE_TRACE_HH__

#include <string>
#include <vector>

#include "base/cprintf.hh"
#include "base/match.hh"
#include "base/traceflags.hh"
#include "sim/host.hh"
#include "sim/core.hh"

namespace Trace {

std::ostream &output();
void setOutput(const std::string &filename);

extern bool enabled;
typedef std::vector<bool> FlagVec;
extern FlagVec flags;
inline bool IsOn(int t) { return flags[t]; }
bool changeFlag(const char *str, bool value);
void dumpStatus();

extern ObjectMatch ignore;
extern const std::string DefaultName;

void dprintf(Tick when, const std::string &name, const char *format,
             CPRINTF_DECLARATION);
void dump(Tick when, const std::string &name, const void *data, int len);

/* namespace Trace */ }

// This silly little class allows us to wrap a string in a functor
// object so that we can give a name() that DPRINTF will like
struct StringWrap
{
    std::string str;
    StringWrap(const std::string &s) : str(s) {}
    const std::string &operator()() const { return str; }
};

inline const std::string &name() { return Trace::DefaultName; }

//
// DPRINTF is a debugging trace facility that allows one to
// selectively enable tracing statements.  To use DPRINTF, there must
// be a function or functor called name() that returns a const
// std::string & in the current scope.
//
// If you desire that the automatic printing not occur, use DPRINTFR
// (R for raw)
//

#if TRACING_ON

#define DTRACE(x) (Trace::IsOn(Trace::x) && Trace::enabled)

#define DDUMP(x, data, count) do {                              \
    if (DTRACE(x))                                              \
        Trace::dump(curTick, name(), data, count);              \
} while (0)

#define DPRINTF(x, ...) do {                                    \
    if (DTRACE(x))                                              \
        Trace::dprintf(curTick, name(), __VA_ARGS__);           \
} while (0)

#define DPRINTFR(x, ...) do {                                   \
    if (DTRACE(x))                                              \
        Trace::dprintf((Tick)-1, std::string(), __VA_ARGS__);   \
} while (0)

#define DDUMPN(data, count) do {                                \
    Trace::dump(curTick, name(), data, count);                  \
} while (0)

#define DPRINTFN(...) do {                                      \
    Trace::dprintf(curTick, name(), __VA_ARGS__);               \
} while (0)

#define DPRINTFNR(...) do {                                     \
    Trace::dprintf((Tick)-1, string(), __VA_ARGS__);            \
} while (0)

#else // !TRACING_ON

#define DTRACE(x) (false)
#define DDUMP(x, data, count) do {} while (0)
#define DPRINTF(x, ...) do {} while (0)
#define DPRINTFR(...) do {} while (0)
#define DDUMPN(data, count) do {} while (0)
#define DPRINTFN(...) do {} while (0)
#define DPRINTFNR(...) do {} while (0)

#endif	// TRACING_ON

#endif // __BASE_TRACE_HH__
