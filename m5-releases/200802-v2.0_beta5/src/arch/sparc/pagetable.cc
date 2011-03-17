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
 */

#include "arch/sparc/pagetable.hh"
#include "sim/serialize.hh"

namespace SparcISA
{
void
TlbEntry::serialize(std::ostream &os)
{
    SERIALIZE_SCALAR(range.va);
    SERIALIZE_SCALAR(range.size);
    SERIALIZE_SCALAR(range.contextId);
    SERIALIZE_SCALAR(range.partitionId);
    SERIALIZE_SCALAR(range.real);
    uint64_t entry4u = 0;
    if (valid)
        entry4u = pte();
    SERIALIZE_SCALAR(entry4u);
    SERIALIZE_SCALAR(used);
    SERIALIZE_SCALAR(valid);
}


void
TlbEntry::unserialize(Checkpoint *cp, const std::string &section)
{
    UNSERIALIZE_SCALAR(range.va);
    UNSERIALIZE_SCALAR(range.size);
    UNSERIALIZE_SCALAR(range.contextId);
    UNSERIALIZE_SCALAR(range.partitionId);
    UNSERIALIZE_SCALAR(range.real);
    uint64_t entry4u;
    UNSERIALIZE_SCALAR(entry4u);
    if (entry4u)
        pte.populate(entry4u);
    UNSERIALIZE_SCALAR(used);
    UNSERIALIZE_SCALAR(valid);
}


int PageTableEntry::pageSizes[] = {8*1024, 64*1024, 0, 4*1024*1024, 0,
            256*1024*1024L};


}
