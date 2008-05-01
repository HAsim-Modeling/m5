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

#include "base/loader/symtab.hh"
#include "cpu/base.hh"
#include "cpu/exetrace.hh"
#include "cpu/static_inst.hh"
#include "cpu/thread_context.hh"
#include "enums/OpClass.hh"

using namespace std;
using namespace TheISA;

namespace Trace {

void
Trace::ExeTracerRecord::dump()
{
    ostream &outs = Trace::output();

    if (IsOn(ExecTicks))
        ccprintf(outs, "%7d: ", when);

    outs << thread->getCpuPtr()->name() << " ";

    if (IsOn(ExecSpeculative))
        outs << (misspeculating ? "-" : "+") << " ";

    if (IsOn(ExecThread))
        outs << "T" << thread->getThreadNum() << " : ";


    std::string sym_str;
    Addr sym_addr;
    outs << "0x" << hex << PC << " : ";
    if (debugSymbolTable
        && IsOn(ExecSymbol)
        && debugSymbolTable->findNearestSymbol(PC, sym_str, sym_addr)) {
        if (PC != sym_addr)
            sym_str += csprintf("+%d", PC - sym_addr);
        outs << "@" << sym_str << " : ";
    }

    //
    //  Print decoded instruction
    //

    outs << setw(26) << left;
    outs << staticInst->disassemble(PC, debugSymbolTable);
    outs << " : ";

    if (IsOn(ExecOpClass)) {
        outs << Enums::OpClassStrings[staticInst->opClass()] << " : ";
    }

    if (IsOn(ExecResult) && data_status != DataInvalid) {
        ccprintf(outs, " D=%#018x", data.as_int);
    }

    if (IsOn(ExecEffAddr) && addr_valid)
        outs << " A=0x" << hex << addr;

    if (IsOn(ExecFetchSeq) && fetch_seq_valid)
        outs << "  FetchSeq=" << dec << fetch_seq;

    if (IsOn(ExecCPSeq) && cp_seq_valid)
        outs << "  CPSeq=" << dec << cp_seq;

    //
    //  End of line...
    //
    outs << endl;
}

/* namespace Trace */ }

////////////////////////////////////////////////////////////////////////
//
//  ExeTracer Simulation Object
//
Trace::ExeTracer *
ExeTracerParams::create()
{
    return new Trace::ExeTracer(this);
};
