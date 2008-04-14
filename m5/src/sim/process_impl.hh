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
 * Authors: Nathan L. Binkert
 *          Steven K. Reinhardt
 */

#ifndef __SIM_PROCESS_IMPL_HH__
#define __SIM_PROCESS_IMPL_HH__

//
// The purpose of this code is to fake the loader & syscall mechanism
// when there's no OS: thus there's no reason to use it in FULL_SYSTEM
// mode when we do have an OS.
//
#include "config/full_system.hh"

#if !FULL_SYSTEM

#include <string>
#include <vector>

#include "mem/translating_port.hh"


//This needs to be templated for cases where 32 bit pointers are needed.
template<class AddrType>
void
copyStringArray(std::vector<std::string> &strings,
        AddrType array_ptr, AddrType data_ptr,
        TranslatingPort* memPort)
{
    AddrType data_ptr_swap;
    for (int i = 0; i < strings.size(); ++i) {
        data_ptr_swap = htog(data_ptr);
        memPort->writeBlob(array_ptr, (uint8_t*)&data_ptr_swap,
                sizeof(AddrType));
        memPort->writeString(data_ptr, strings[i].c_str());
        array_ptr += sizeof(AddrType);
        data_ptr += strings[i].size() + 1;
    }
    // add NULL terminator
    data_ptr = 0;

    memPort->writeBlob(array_ptr, (uint8_t*)&data_ptr, sizeof(AddrType));
}


#endif // !FULL_SYSTEM

#endif
