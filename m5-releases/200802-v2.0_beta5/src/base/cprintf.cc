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
 */

#include <cassert>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "base/cprintf.hh"

using namespace std;

namespace cp {

Print::Print(std::ostream &stream, const std::string &format)
    : stream(stream), format(format.c_str()), ptr(format.c_str())
{
    saved_flags = stream.flags();
    saved_fill = stream.fill();
    saved_precision = stream.precision();
}

Print::Print(std::ostream &stream, const char *format)
    : stream(stream), format(format), ptr(format)
{
    saved_flags = stream.flags();
    saved_fill = stream.fill();
    saved_precision = stream.precision();
}

Print::~Print()
{
}

void
Print::process(Format &fmt)
{
    size_t len;

    while (*ptr) {
        switch (*ptr) {
          case '%':
            if (ptr[1] != '%')
                goto processing;

            stream.put('%');
            ptr += 2;
            break;

          case '\n':
            stream << endl;
            ++ptr;
            break;
          case '\r':
            ++ptr;
            if (*ptr != '\n')
                stream << endl;
            break;

          default:
            len = strcspn(ptr, "%\n\r\0");
            stream.write(ptr, len);
            ptr += len;
            break;
        }
    }

    return;

  processing:
    bool done = false;
    bool end_number = false;
    bool have_precision = false;
    int number = 0;

    stream.fill(' ');
    stream.flags((ios::fmtflags)0);

    while (!done) {
        ++ptr;
        if (*ptr >= '0' && *ptr <= '9') {
            if (end_number)
                continue;
        } else if (number > 0)
            end_number = true;

        switch (*ptr) {
          case 's':
            fmt.format = Format::string;
            done = true;
            break;

          case 'c':
            fmt.format = Format::character;
            done = true;
            break;

          case 'l':
            continue;

          case 'p':
            fmt.format = Format::integer;
            fmt.base = Format::hex;
            fmt.alternate_form = true;
            done = true;
            break;

          case 'X':
            fmt.uppercase = true;
          case 'x':
            fmt.base = Format::hex;
            fmt.format = Format::integer;
            done = true;
            break;

          case 'o':
            fmt.base = Format::oct;
            fmt.format = Format::integer;
            done = true;
            break;

          case 'd':
          case 'i':
          case 'u':
            fmt.format = Format::integer;
            done = true;
            break;

          case 'G':
            fmt.uppercase = true;
          case 'g':
            fmt.format = Format::floating;
            fmt.float_format = Format::best;
            done = true;
            break;

          case 'E':
            fmt.uppercase = true;
          case 'e':
            fmt.format = Format::floating;
            fmt.float_format = Format::scientific;
            done = true;
            break;

          case 'f':
            fmt.format = Format::floating;
            fmt.float_format = Format::fixed;
            done = true;
            break;

          case 'n':
            stream << "we don't do %n!!!\n";
            done = true;
            break;

          case '#':
            fmt.alternate_form = true;
            break;

          case '-':
            fmt.flush_left = true;
            break;

          case '+':
            fmt.print_sign = true;
            break;

          case ' ':
            fmt.blank_space = true;
            break;

          case '.':
            fmt.width = number;
            fmt.precision = 0;
            have_precision = true;
            number = 0;
            end_number = false;
            break;

          case '0':
            if (number == 0) {
                fmt.fill_zero = true;
                break;
            }
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
            number = number * 10 + (*ptr - '0');
            break;

          case '%':
            assert("we shouldn't get here");
            break;

          default:
            done = true;
            break;
        }

        if (end_number) {
            if (have_precision)
                fmt.precision = number;
            else
                fmt.width = number;

            end_number = false;
            number = 0;
        }
    }

    ++ptr;
}

void
Print::end_args()
{
    size_t len;

    while (*ptr) {
        switch (*ptr) {
          case '%':
            if (ptr[1] != '%')
                stream << "<extra arg>";

            stream.put('%');
            ptr += 2;
            break;

          case '\n':
            stream << endl;
            ++ptr;
            break;
          case '\r':
            ++ptr;
            if (*ptr != '\n')
                stream << endl;
            break;

          default:
            len = strcspn(ptr, "%\n\r\0");
            stream.write(ptr, len);
            ptr += len;
            break;
        }
    }

    stream.flags(saved_flags);
    stream.fill(saved_fill);
    stream.precision(saved_precision);
}

/* end namespace cp */ }
