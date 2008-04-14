/*
 * Copyright (c) 2001, 2002, 2003, 2004, 2005
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
 * Authors: Steven K. Reinhardt
 *          Lisa R. Hsu
 *          Nathan L. Binkert
 *          Steven E. Raasch
 */

#include <iomanip>

#include "cpu/exetrace.hh"
#include "cpu/inteltrace.hh"
#include "cpu/static_inst.hh"

using namespace std;
using namespace TheISA;

namespace Trace {

void
Trace::IntelTraceRecord::dump()
{
    ostream &outs = Trace::output();
    ccprintf(outs, "%7d ) ", when);
    outs << "0x" << hex << PC << ":\t";
    if (staticInst->isLoad()) {
        ccprintf(outs, "<RD %#x>", addr);
    } else if (staticInst->isStore()) {
        ccprintf(outs, "<WR %#x>", addr);
    }
    outs << endl;
}

/* namespace Trace */ }

////////////////////////////////////////////////////////////////////////
//
//  ExeTracer Simulation Object
//
Trace::IntelTrace *
IntelTraceParams::create()
{
    return new Trace::IntelTrace(this);
};
