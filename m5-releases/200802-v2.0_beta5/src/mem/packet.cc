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
 * Authors: Ali G. Saidi
 *          Steven K. Reinhardt
 */

/**
 * @file
 * Definition of the Packet Class, a packet is a transaction occuring
 * between a single level of the memory heirarchy (ie L1->L2).
 */

#include <iostream>
#include <cstring>
#include "base/cprintf.hh"
#include "base/misc.hh"
#include "base/trace.hh"
#include "mem/packet.hh"

// The one downside to bitsets is that static initializers can get ugly.
#define SET1(a1)                     (1 << (a1))
#define SET2(a1, a2)                 (SET1(a1) | SET1(a2))
#define SET3(a1, a2, a3)             (SET2(a1, a2) | SET1(a3))
#define SET4(a1, a2, a3, a4)         (SET3(a1, a2, a3) | SET1(a4))
#define SET5(a1, a2, a3, a4, a5)     (SET4(a1, a2, a3, a4) | SET1(a5))
#define SET6(a1, a2, a3, a4, a5, a6) (SET5(a1, a2, a3, a4, a5) | SET1(a6))

const MemCmd::CommandInfo
MemCmd::commandInfo[] =
{
    /* InvalidCmd */
    { 0, InvalidCmd, "InvalidCmd" },
    /* ReadReq */
    { SET3(IsRead, IsRequest, NeedsResponse), ReadResp, "ReadReq" },
    /* ReadResp */
    { SET3(IsRead, IsResponse, HasData), InvalidCmd, "ReadResp" },
    /* ReadRespWithInvalidate */
    { SET4(IsRead, IsResponse, HasData, IsInvalidate),
            InvalidCmd, "ReadRespWithInvalidate" },
    /* WriteReq */
    { SET5(IsWrite, NeedsExclusive, IsRequest, NeedsResponse, HasData),
            WriteResp, "WriteReq" },
    /* WriteResp */
    { SET3(IsWrite, NeedsExclusive, IsResponse), InvalidCmd, "WriteResp" },
    /* Writeback */
    { SET4(IsWrite, NeedsExclusive, IsRequest, HasData),
            InvalidCmd, "Writeback" },
    /* SoftPFReq */
    { SET4(IsRead, IsRequest, IsSWPrefetch, NeedsResponse),
            SoftPFResp, "SoftPFReq" },
    /* HardPFReq */
    { SET4(IsRead, IsRequest, IsHWPrefetch, NeedsResponse),
            HardPFResp, "HardPFReq" },
    /* SoftPFResp */
    { SET4(IsRead, IsResponse, IsSWPrefetch, HasData),
            InvalidCmd, "SoftPFResp" },
    /* HardPFResp */
    { SET4(IsRead, IsResponse, IsHWPrefetch, HasData),
            InvalidCmd, "HardPFResp" },
    /* WriteInvalidateReq */
    { SET6(IsWrite, NeedsExclusive, IsInvalidate,
           IsRequest, HasData, NeedsResponse),
            WriteInvalidateResp, "WriteInvalidateReq" },
    /* WriteInvalidateResp */
    { SET3(IsWrite, NeedsExclusive, IsResponse),
            InvalidCmd, "WriteInvalidateResp" },
    /* UpgradeReq */
    { SET4(IsInvalidate, NeedsExclusive, IsRequest, NeedsResponse),
            UpgradeResp, "UpgradeReq" },
    /* UpgradeResp */
    { SET2(NeedsExclusive, IsResponse),
            InvalidCmd, "UpgradeResp" },
    /* ReadExReq */
    { SET5(IsRead, NeedsExclusive, IsInvalidate, IsRequest, NeedsResponse),
            ReadExResp, "ReadExReq" },
    /* ReadExResp */
    { SET4(IsRead, NeedsExclusive, IsResponse, HasData),
            InvalidCmd, "ReadExResp" },
    /* LoadLockedReq */
    { SET4(IsRead, IsLocked, IsRequest, NeedsResponse),
            LoadLockedResp, "LoadLockedReq" },
    /* LoadLockedResp */
    { SET4(IsRead, IsLocked, IsResponse, HasData),
            InvalidCmd, "LoadLockedResp" },
    /* StoreCondReq */
    { SET6(IsWrite, NeedsExclusive, IsLocked,
           IsRequest, NeedsResponse, HasData),
            StoreCondResp, "StoreCondReq" },
    /* StoreCondResp */
    { SET4(IsWrite, NeedsExclusive, IsLocked, IsResponse),
            InvalidCmd, "StoreCondResp" },
    /* SwapReq -- for Swap ldstub type operations */
    { SET6(IsRead, IsWrite, NeedsExclusive, IsRequest, HasData, NeedsResponse),
        SwapResp, "SwapReq" },
    /* SwapResp -- for Swap ldstub type operations */
    { SET5(IsRead, IsWrite, NeedsExclusive, IsResponse, HasData),
            InvalidCmd, "SwapResp" },
    /* NetworkNackError  -- nacked at network layer (not by protocol) */
    { SET2(IsResponse, IsError), InvalidCmd, "NetworkNackError" },
    /* InvalidDestError  -- packet dest field invalid */
    { SET2(IsResponse, IsError), InvalidCmd, "InvalidDestError" },
    /* BadAddressError   -- memory address invalid */
    { SET2(IsResponse, IsError), InvalidCmd, "BadAddressError" },
    /* PrintReq */
    { SET2(IsRequest, IsPrint), InvalidCmd, "PrintReq" }
};


/** delete the data pointed to in the data pointer. Ok to call to matter how
 * data was allocted. */
void
Packet::deleteData()
{
    assert(staticData || dynamicData);
    if (staticData)
        return;

    if (arrayData)
        delete [] data;
    else
        delete data;
}

/** If there isn't data in the packet, allocate some. */
void
Packet::allocate()
{
    if (data)
        return;
    assert(!staticData);
    dynamicData = true;
    arrayData = true;
    data = new uint8_t[getSize()];
}


bool
Packet::checkFunctional(Printable *obj, Addr addr, int size, uint8_t *data)
{
    Addr func_start = getAddr();
    Addr func_end   = getAddr() + getSize() - 1;
    Addr val_start  = addr;
    Addr val_end    = val_start + size - 1;

    if (func_start > val_end || val_start > func_end) {
        // no intersection
        return false;
    }

    // check print first since it doesn't require data
    if (isPrint()) {
        dynamic_cast<PrintReqState*>(senderState)->printObj(obj);
        return false;
    }

    // if there's no data, there's no need to look further
    if (!data) {
        return false;
    }

    // offset of functional request into supplied value (could be
    // negative if partial overlap)
    int offset = func_start - val_start;

    if (isRead()) {
        if (func_start >= val_start && func_end <= val_end) {
            allocate();
            std::memcpy(getPtr<uint8_t>(), data + offset, getSize());
            makeResponse();
            return true;
        } else {
            // In this case the timing packet only partially satisfies
            // the request, so we would need more information to make
            // this work.  Like bytes valid in the packet or
            // something, so the request could continue and get this
            // bit of possibly newer data along with the older data
            // not written to yet.
            panic("Memory value only partially satisfies the functional "
                  "request. Now what?");
        }
    } else if (isWrite()) {
        if (offset >= 0) {
            std::memcpy(data + offset, getPtr<uint8_t>(),
                        (std::min(func_end, val_end) - func_start) + 1);
        } else { // val_start > func_start
            std::memcpy(data, getPtr<uint8_t>() - offset,
                        (std::min(func_end, val_end) - val_start) + 1);
        }
    } else {
        panic("Don't know how to handle command %s\n", cmdString());
    }

    // keep going with request by default
    return false;
}


void
Packet::print(std::ostream &o, const int verbosity,
              const std::string &prefix) const
{
    ccprintf(o, "%s[%x:%x] %s\n", prefix,
             getAddr(), getAddr() + getSize() - 1, cmdString());
}


Packet::PrintReqState::PrintReqState(std::ostream &_os, int _verbosity)
    : curPrefixPtr(new std::string("")), os(_os), verbosity(_verbosity)
{
    labelStack.push_back(LabelStackEntry("", curPrefixPtr));
}


Packet::PrintReqState::~PrintReqState()
{
    labelStack.pop_back();
    assert(labelStack.empty());
    delete curPrefixPtr;
}


Packet::PrintReqState::
LabelStackEntry::LabelStackEntry(const std::string &_label,
                                 std::string *_prefix)
    : label(_label), prefix(_prefix), labelPrinted(false)
{
}


void
Packet::PrintReqState::pushLabel(const std::string &lbl,
                                 const std::string &prefix)
{
    labelStack.push_back(LabelStackEntry(lbl, curPrefixPtr));
    curPrefixPtr = new std::string(*curPrefixPtr);
    *curPrefixPtr += prefix;
}

void
Packet::PrintReqState::popLabel()
{
    delete curPrefixPtr;
    curPrefixPtr = labelStack.back().prefix;
    labelStack.pop_back();
    assert(!labelStack.empty());
}

void
Packet::PrintReqState::printLabels()
{
    if (!labelStack.back().labelPrinted) {
        LabelStack::iterator i = labelStack.begin();
        LabelStack::iterator end = labelStack.end();
        while (i != end) {
            if (!i->labelPrinted) {
                ccprintf(os, "%s%s\n", *(i->prefix), i->label);
                i->labelPrinted = true;
            }
            i++;
        }
    }
}


void
Packet::PrintReqState::printObj(Printable *obj)
{
    printLabels();
    obj->print(os, verbosity, curPrefix());
}
