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
 *          David A. Greene
 */

#ifndef __MISC_HH__
#define __MISC_HH__

#include <cassert>

#include "base/compiler.hh"
#include "base/cprintf.hh"
#include "base/varargs.hh"

#if defined(__SUNPRO_CC)
#define __FUNCTION__ "how to fix me?"
#endif

//
// This implements a cprintf based panic() function.  panic() should
// be called when something happens that should never ever happen
// regardless of what the user does (i.e., an acutal m5 bug).  panic()
// calls abort which can dump core or enter the debugger.
//
//
void __panic(const char *func, const char *file, int line, const char *format,
             CPRINTF_DECLARATION) M5_ATTR_NORETURN;
void __panic(const char *func, const char *file, int line,
             const std::string &format, CPRINTF_DECLARATION)
M5_ATTR_NORETURN;

inline void
__panic(const char *func, const char *file, int line,
        const std::string &format, CPRINTF_DEFINITION)
{
    __panic(func, file, line, format.c_str(), VARARGS_ALLARGS);
}
M5_PRAGMA_NORETURN(__panic)
#define panic(...) __panic(__FUNCTION__, __FILE__, __LINE__, __VA_ARGS__)

//
// This implements a cprintf based fatal() function.  fatal() should
// be called when the simulation cannot continue due to some condition
// that is the user's fault (bad configuration, invalid arguments,
// etc.) and not a simulator bug.  fatal() calls exit(1), i.e., a
// "normal" exit with an error code, as opposed to abort() like
// panic() does.
//
void __fatal(const char *func, const char *file, int line, const char *format,
             CPRINTF_DECLARATION) M5_ATTR_NORETURN;
void __fatal(const char *func, const char *file, int line,
             const std::string &format, CPRINTF_DECLARATION)
    M5_ATTR_NORETURN;

inline void
__fatal(const char *func, const char *file, int line,
        const std::string &format, CPRINTF_DEFINITION)
{
    __fatal(func, file, line, format.c_str(), VARARGS_ALLARGS);
}
M5_PRAGMA_NORETURN(__fatal)
#define fatal(...) __fatal(__FUNCTION__, __FILE__, __LINE__, __VA_ARGS__)

//
// This implements a cprintf based warn
//
void __warn(const char *func, const char *file, int line, const char *format,
            CPRINTF_DECLARATION);
inline void
__warn(const char *func, const char *file, int line, const std::string &format,
       CPRINTF_DECLARATION)
{
    __warn(func, file, line, format, VARARGS_ALLARGS);
}
#define warn(...) __warn(__FUNCTION__, __FILE__, __LINE__, __VA_ARGS__)

// Only print the warning message the first time it is seen.  This
// doesn't check the warning string itself, it just only lets one
// warning come from the statement. So, even if the arguments change
// and that would have resulted in a different warning message,
// subsequent messages would still be supressed.
#define warn_once(...) do {                         \
        static bool once = false;                   \
        if (!once) {                                \
            warn(__VA_ARGS__);                       \
            once = true;                            \
        }                                           \
    } while (0)

//
// assert() that prints out the current cycle
//
#define m5_assert(TEST) do {                                            \
    if (!(TEST))                                                        \
        ccprintf(std::cerr, "Assertion failure, curTick = %d\n", curTick); \
    assert(TEST);                                                       \
} while (0)

#endif // __MISC_HH__
