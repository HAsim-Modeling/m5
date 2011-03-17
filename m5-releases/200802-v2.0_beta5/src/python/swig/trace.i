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

%module trace

%{
#include "base/trace.hh"
#include "sim/host.hh"

inline void
output(const char *filename)
{
    Trace::setOutput(filename);
}

inline void
set(const char *flag)
{
    Trace::changeFlag(flag, true);
}

inline void
clear(const char *flag)
{
    Trace::changeFlag(flag, false);
}

inline void
ignore(const char *expr)
{
    Trace::ignore.setExpression(expr);
}

using Trace::enabled;
%}

%inline %{
extern void output(const char *string);
extern void set(const char *string);
extern void clear(const char *string);
extern void ignore(const char *expr);
extern bool enabled;
%}

%wrapper %{
// fix up module name to reflect the fact that it's inside the m5 package
#undef SWIG_name
#define SWIG_name "m5.internal._trace"
%}
