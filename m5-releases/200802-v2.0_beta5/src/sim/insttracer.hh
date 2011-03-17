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
 *          Nathan L. Binkert
 */

#ifndef __INSTRECORD_HH__
#define __INSTRECORD_HH__

#include "base/bigint.hh"
#include "base/trace.hh"
#include "cpu/inst_seq.hh"	// for InstSeqNum
#include "cpu/static_inst.hh"
#include "sim/host.hh"
#include "sim/sim_object.hh"

class ThreadContext;

namespace Trace {

class InstRecord
{
  protected:
    Tick when;

    // The following fields are initialized by the constructor and
    // thus guaranteed to be valid.
    ThreadContext *thread;
    // need to make this ref-counted so it doesn't go away before we
    // dump the record
    StaticInstPtr staticInst;
    Addr PC;
    bool misspeculating;

    // The remaining fields are only valid for particular instruction
    // types (e.g, addresses for memory ops) or when particular
    // options are enabled (e.g., tracing full register contents).
    // Each data field has an associated valid flag to indicate
    // whether the data field is valid.
    Addr addr;
    bool addr_valid;

    union {
        uint64_t as_int;
        double as_double;
    } data;
    enum {
        DataInvalid = 0,
        DataInt8 = 1,	// set to equal number of bytes
        DataInt16 = 2,
        DataInt32 = 4,
        DataInt64 = 8,
        DataDouble = 3
    } data_status;

    InstSeqNum fetch_seq;
    bool fetch_seq_valid;

    InstSeqNum cp_seq;
    bool cp_seq_valid;

  public:
    InstRecord(Tick _when, ThreadContext *_thread,
               const StaticInstPtr &_staticInst,
               Addr _pc, bool spec)
        : when(_when), thread(_thread),
          staticInst(_staticInst), PC(_pc),
          misspeculating(spec)
    {
        data_status = DataInvalid;
        addr_valid = false;

        fetch_seq_valid = false;
        cp_seq_valid = false;
    }

    virtual ~InstRecord() { }

    void setAddr(Addr a) { addr = a; addr_valid = true; }

    void setData(Twin64_t d) { data.as_int = d.a; data_status = DataInt64; }
    void setData(Twin32_t d) { data.as_int = d.a; data_status = DataInt32; }
    void setData(uint64_t d) { data.as_int = d; data_status = DataInt64; }
    void setData(uint32_t d) { data.as_int = d; data_status = DataInt32; }
    void setData(uint16_t d) { data.as_int = d; data_status = DataInt16; }
    void setData(uint8_t d) { data.as_int = d; data_status = DataInt8; }

    void setData(int64_t d) { setData((uint64_t)d); }
    void setData(int32_t d) { setData((uint32_t)d); }
    void setData(int16_t d) { setData((uint16_t)d); }
    void setData(int8_t d)  { setData((uint8_t)d); }

    void setData(double d) { data.as_double = d; data_status = DataDouble; }

    void setFetchSeq(InstSeqNum seq)
    { fetch_seq = seq; fetch_seq_valid = true; }

    void setCPSeq(InstSeqNum seq)
    { cp_seq = seq; cp_seq_valid = true; }

    virtual void dump() = 0;
};

class InstTracer : public SimObject
{
  public:
    InstTracer(const Params *p) : SimObject(p)
    {}

    virtual ~InstTracer()
    {};

    virtual InstRecord *
        getInstRecord(Tick when, ThreadContext *tc,
                const StaticInstPtr staticInst, Addr pc) = 0;
};



}; // namespace Trace

#endif // __INSTRECORD_HH__
