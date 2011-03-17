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
 *          Ali G. Saidi
 */

#include <sys/types.h>
#include <algorithm>

#include "sim/arguments.hh"
#include "base/trace.hh"
#include "kern/linux/printk.hh"

using namespace std;


void
Printk(stringstream &out, Arguments args)
{
    char *p = (char *)args++;

    while (*p) {
        switch (*p) {
          case '%': {
              bool more = true;
              bool islong = false;
              bool leftjustify = false;
              bool format = false;
              bool zero = false;
              int width = 0;
              while (more && *++p) {
                  switch (*p) {
                    case 'l':
                    case 'L':
                      islong = true;
                      break;
                    case '-':
                      leftjustify = true;
                      break;
                    case '#':
                      format = true;
                      break;
                    case '0':
                      if (width)
                          width *= 10;
                      else
                          zero = true;
                      break;
                    default:
                      if (*p >= '1' && *p <= '9')
                          width = 10 * width + *p - '0';
                      else
                          more = false;
                      break;
                  }
              }

              bool hexnum = false;
              bool octal = false;
              bool sign = false;
              switch (*p) {
                case 'X':
                case 'x':
                  hexnum = true;
                  break;
                case 'O':
                case 'o':
                  octal = true;
                  break;
                case 'D':
                case 'd':
                  sign = true;
                  break;
                case 'P':
                  format = true;
                case 'p':
                  hexnum = true;
                  break;
              }

              switch (*p) {
                case 'D':
                case 'd':
                case 'U':
                case 'u':
                case 'X':
                case 'x':
                case 'O':
                case 'o':
                case 'P':
                case 'p': {
                  if (hexnum)
                      out << hex;

                  if (octal)
                      out << oct;

                  if (format) {
                      if (!zero)
                          out.setf(ios::showbase);
                      else {
                          if (hexnum) {
                              out << "0x";
                              width -= 2;
                          } else if (octal) {
                              out << "0";
                              width -= 1;
                          }
                      }
                  }

                  if (zero)
                      out.fill('0');

                  if (width > 0)
                      out.width(width);

                  if (leftjustify && !zero)
                      out.setf(ios::left);

                  if (sign) {
                      if (islong)
                          out << (int64_t)args;
                      else
                          out << (int32_t)args;
                  } else {
                      if (islong)
                          out << (uint64_t)args;
                      else
                          out << (uint32_t)args;
                  }

                  if (zero)
                      out.fill(' ');

                  if (width > 0)
                      out.width(0);

                  out << dec;

                  ++args;
                }
                  break;

                case 's': {
                    const char *s = (const char *)args;
                    if (!s)
                        s = "<NULL>";

                    if (width > 0)
                        out.width(width);
                    if (leftjustify)
                        out.setf(ios::left);

                    out << s;
                    ++args;
                }
                  break;
                case 'C':
                case 'c': {
                    uint64_t mask = (*p == 'C') ? 0xffL : 0x7fL;
                    uint64_t num;
                    int width;

                    if (islong) {
                        num = (uint64_t)args;
                        width = sizeof(uint64_t);
                    } else {
                        num = (uint32_t)args;
                        width = sizeof(uint32_t);
                    }

                    while (width-- > 0) {
                        char c = (char)(num & mask);
                        if (c)
                            out << c;
                        num >>= 8;
                    }

                    ++args;
                }
                  break;
                case 'b': {
                  uint64_t n = (uint64_t)args++;
                  char *s = (char *)args++;
                  out << s << ": " << n;
                }
                  break;
                case 'n':
                case 'N': {
                    args += 2;
#if 0
                    uint64_t n = (uint64_t)args++;
                    struct reg_values *rv = (struct reg_values *)args++;
#endif
                }
                  break;
                case 'r':
                case 'R': {
                    args += 2;
#if 0
                    uint64_t n = (uint64_t)args++;
                    struct reg_desc *rd = (struct reg_desc *)args++;
#endif
                }
                  break;
                case '%':
                  out << '%';
                  break;
              }
              ++p;
          }
            break;
          case '\n':
            out << endl;
            ++p;
            break;
          case '\r':
            ++p;
            if (*p != '\n')
                out << endl;
            break;

          default: {
              size_t len = strcspn(p, "%\n\r\0");
              out.write(p, len);
              p += len;
          }
        }
    }

}

