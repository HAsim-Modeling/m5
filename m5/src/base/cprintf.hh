/*
 * Copyright (c) 2002, 2003, 2004, 2005, 2006
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

#ifndef __BASE_CPRINTF_HH__
#define __BASE_CPRINTF_HH__

#include <ios>
#include <iostream>
#include <list>
#include <string>

#include "base/varargs.hh"
#include "base/cprintf_formats.hh"

namespace cp {

#define CPRINTF_DECLARATION VARARGS_DECLARATION(cp::Print)
#define CPRINTF_DEFINITION VARARGS_DEFINITION(cp::Print)

struct Print
{
  protected:
    std::ostream &stream;
    const char *format;
    const char *ptr;

    std::ios::fmtflags saved_flags;
    char saved_fill;
    int saved_precision;

    void process(Format &fmt);

  public:
    Print(std::ostream &stream, const std::string &format);
    Print(std::ostream &stream, const char *format);
    ~Print();

    template <typename T>
    void
    add_arg(const T &data)
    {
        Format fmt;
        process(fmt);

        switch (fmt.format) {
          case Format::character:
            format_char(stream, data, fmt);
            break;

          case Format::integer:
            format_integer(stream, data, fmt);
            break;

          case Format::floating:
            format_float(stream, data, fmt);
            break;

          case Format::string:
            format_string(stream, data, fmt);
            break;

          default:
            stream << "<bad format>";
            break;
        }
    }

    void end_args();
};

/* end namespace cp */ }

typedef VarArgs::List<cp::Print> CPrintfArgsList;

inline void
ccprintf(std::ostream &stream, const char *format, const CPrintfArgsList &args)
{
    cp::Print print(stream, format);
    args.add_args(print);
}

inline void
ccprintf(std::ostream &stream, const char *format, CPRINTF_DECLARATION)
{
    cp::Print print(stream, format);
    VARARGS_ADDARGS(print);
}

inline void
cprintf(const char *format, CPRINTF_DECLARATION)
{
    ccprintf(std::cout, format, VARARGS_ALLARGS);
}

inline std::string
csprintf(const char *format, CPRINTF_DECLARATION)
{
    std::stringstream stream;
    ccprintf(stream, format, VARARGS_ALLARGS);
    return stream.str();
}

/*
 * functions again with std::string.  We have both so we don't waste
 * time converting const char * to std::string since we don't take
 * advantage of it.
 */
inline void
ccprintf(std::ostream &stream, const std::string &format,
         const CPrintfArgsList &args)
{
    ccprintf(stream, format.c_str(), args);
}

inline void
ccprintf(std::ostream &stream, const std::string &format, CPRINTF_DECLARATION)
{
    ccprintf(stream, format.c_str(), VARARGS_ALLARGS);
}

inline void
cprintf(const std::string &format, CPRINTF_DECLARATION)
{
    ccprintf(std::cout, format.c_str(), VARARGS_ALLARGS);
}

inline std::string
csprintf(const std::string &format, CPRINTF_DECLARATION)
{
    std::stringstream stream;
    ccprintf(stream, format.c_str(), VARARGS_ALLARGS);
    return stream.str();
}

#endif // __CPRINTF_HH__
