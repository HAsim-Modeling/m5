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

#include <ctype.h>
#include <fstream>
#include <iostream>
#include <list>
#include <string>
#include <vector>

#include "base/misc.hh"
#include "base/output.hh"
#include "base/str.hh"
#include "base/trace.hh"
#include "base/varargs.hh"

using namespace std;

namespace Trace {
const string DefaultName("global");
FlagVec flags(NumFlags, false);
bool enabled = false;

//
// This variable holds the output stream for debug information.  Other
// than setting up/redirecting this stream, do *NOT* reference this
// directly; use DebugOut() (see below) to access this stream for
// output.
//
ostream *dprintf_stream = &cerr;
ostream &
output()
{
    return *dprintf_stream;
}

void
setOutput(const string &filename)
{
    dprintf_stream = simout.find(filename);
}

ObjectMatch ignore;

void
dprintf(Tick when, const std::string &name, const char *format,
        CPRINTF_DEFINITION)
{
    if (!name.empty() && ignore.match(name))
        return;

    std::ostream &os = *dprintf_stream;

    string fmt = "";
    CPrintfArgsList args(VARARGS_ALLARGS);

    if (!name.empty()) {
        fmt = "%s: " + fmt;
        args.push_front(name);
    }

    if (when != (Tick)-1) {
        fmt = "%7d: " + fmt;
        args.push_front(when);
    }

    fmt += format;

    ccprintf(os, fmt.c_str(), args);
    os.flush();
}

void
dump(Tick when, const std::string &name, const void *d, int len)
{
    if (!name.empty() && ignore.match(name))
        return;

    std::ostream &os = *dprintf_stream;

    string fmt = "";
    CPrintfArgsList args;

    if (!name.empty()) {
        fmt = "%s: " + fmt;
        args.push_front(name);
    }

    if (when != (Tick)-1) {
        fmt = "%7d: " + fmt;
        args.push_front(when);
    }

    const char *data = static_cast<const char *>(d);
    int c, i, j;
    for (i = 0; i < len; i += 16) {
        ccprintf(os, fmt, args);
        ccprintf(os, "%08x  ", i);
        c = len - i;
        if (c > 16) c = 16;

        for (j = 0; j < c; j++) {
            ccprintf(os, "%02x ", data[i + j] & 0xff);
            if ((j & 0xf) == 7 && j > 0)
                ccprintf(os, " ");
        }

        for (; j < 16; j++)
            ccprintf(os, "   ");
        ccprintf(os, "  ");

        for (j = 0; j < c; j++) {
            int ch = data[i + j] & 0x7f;
            ccprintf(os, "%c", (char)(isprint(ch) ? ch : ' '));
        }

        ccprintf(os, "\n");

        if (c < 16)
            break;
    }
}

bool
changeFlag(const char *s, bool value)
{
    using namespace Trace;
    std::string str(s);

    for (int i = 0; i < numFlagStrings; ++i) {
        if (str != flagStrings[i])
            continue;

        if (i < NumFlags) {
            flags[i] = value;
        } else {
            i -= NumFlags;

            const Flags *flagVec = compoundFlags[i];
            for (int j = 0; flagVec[j] != -1; ++j) {
                if (flagVec[j] < NumFlags)
                    flags[flagVec[j]] = value;
            }
        }

        return true;
    }

    // the flag was not found.
    return false;
}

void
dumpStatus()
{
    using namespace Trace;
    for (int i = 0; i < numFlagStrings; ++i) {
        if (flags[i])
            cprintf("%s\n", flagStrings[i]);
    }
}

/* namespace Trace */ }


// add a set of functions that can easily be invoked from gdb
void
setTraceFlag(const char *string)
{
    Trace::changeFlag(string, true);
}

void
clearTraceFlag(const char *string)
{
    Trace::changeFlag(string, false);
}

void
dumpTraceStatus()
{
    Trace::dumpStatus();
}
