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

/**
 * @file
 * Virtual Port Object Declaration. These ports incorporate some translation
 * into their access methods. Thus you can use one to read and write data
 * to/from virtual addresses.
 */

#ifndef __MEM_VPORT_HH__
#define __MEM_VPORT_HH__

#include "mem/port_impl.hh"
#include "config/full_system.hh"
#include "arch/vtophys.hh"


/** A class that translates a virtual address to a physical address and then
 * calls the above read/write functions. If a thread context is provided the
 * address can alway be translated, If not it can only be translated if it is a
 * simple address masking operation (such as alpha super page accesses).
 */


class VirtualPort  : public FunctionalPort
{
  private:
    ThreadContext *tc;

  public:
    VirtualPort(const std::string &_name, ThreadContext *_tc = NULL)
        : FunctionalPort(_name), tc(_tc)
    {}

    /** Return true if we have an thread context. This is used to
     * prevent someone from accidently deleting the cpus statically
     * allocated vport.
     * @return true if a thread context isn't valid
     */
    bool nullThreadContext() { return tc != NULL; }

    /** Version of readblob that translates virt->phys and deals
      * with page boundries. */
    virtual void readBlob(Addr addr, uint8_t *p, int size);

    /** Version of writeBlob that translates virt->phys and deals
      * with page boundries. */
    virtual void writeBlob(Addr addr, uint8_t *p, int size);
};


void CopyOut(ThreadContext *tc, void *dest, Addr src, size_t cplen);
void CopyIn(ThreadContext *tc, Addr dest, void *source, size_t cplen);
void CopyStringOut(ThreadContext *tc, char *dst, Addr vaddr, size_t maxlen);
void CopyStringIn(ThreadContext *tc, char *src, Addr vaddr);

#endif //__MEM_VPORT_HH__

