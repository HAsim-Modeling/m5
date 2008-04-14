/*
 * Copyright (c) 2002, 2003, 2004, 2005
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

#include <iostream>
#include <string>

#include "base/cprintf.hh"
#include "base/hostinfo.hh"
#include "base/misc.hh"
#include "base/output.hh"
#include "base/trace.hh"
#include "base/varargs.hh"
#include "sim/host.hh"
#include "sim/core.hh"

using namespace std;

void
__panic(const char *func, const char *file, int line, const char *fmt,
    CPRINTF_DEFINITION)
{
    string format = "panic: ";
    format += fmt;
    switch (format[format.size() - 1]) {
      case '\n':
      case '\r':
        break;
      default:
        format += "\n";
    }

    format += " @ cycle %d\n[%s:%s, line %d]\n";

    CPrintfArgsList args(VARARGS_ALLARGS);

    args.push_back(curTick);
    args.push_back(func);
    args.push_back(file);
    args.push_back(line);

    ccprintf(cerr, format.c_str(), args);

    abort();
}

void
__fatal(const char *func, const char *file, int line, const char *fmt,
    CPRINTF_DEFINITION)
{
    CPrintfArgsList args(VARARGS_ALLARGS);
    string format = "fatal: ";
    format += fmt;

    switch (format[format.size() - 1]) {
      case '\n':
      case '\r':
        break;
      default:
        format += "\n";
    }

    format += " @ cycle %d\n[%s:%s, line %d]\n";
    format += "Memory Usage: %ld KBytes\n";

    args.push_back(curTick);
    args.push_back(func);
    args.push_back(file);
    args.push_back(line);
    args.push_back(memUsage());

    ccprintf(cerr, format.c_str(), args);

    exit(1);
}

void
__warn(const char *func, const char *file, int line, const char *fmt,
    CPRINTF_DEFINITION)
{
    string format = "warn: ";
    format += fmt;

    switch (format[format.size() - 1]) {
      case '\n':
      case '\r':
        break;
      default:
        format += "\n";
    }

#ifdef VERBOSE_WARN
    format += " @ cycle %d\n[%s:%s, line %d]\n";
#endif

    CPrintfArgsList args(VARARGS_ALLARGS);

#ifdef VERBOSE_WARN
    args.push_back(curTick);
    args.push_back(func);
    args.push_back(file);
    args.push_back(line);
#endif

    ccprintf(cerr, format.c_str(), args);
    if (simout.isFile(*outputStream))
        ccprintf(*outputStream, format.c_str(), args);
}
