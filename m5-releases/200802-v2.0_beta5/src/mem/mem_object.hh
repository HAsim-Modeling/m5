/*
 * Copyright (c) 2002, 2003, 2004, 2005
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
 * Authors: Ronald G. Dreslinski Jr
 */

/**
 * @file
 * Base Memory Object declaration.
 */

#ifndef __MEM_MEM_OBJECT_HH__
#define __MEM_MEM_OBJECT_HH__

#include "mem/port.hh"
#include "params/MemObject.hh"
#include "sim/sim_object.hh"

/**
 * The base MemoryObject class, allows for an accesor function to a
 * simobj that returns the Port.
 */
class MemObject : public SimObject
{
  public:
    typedef MemObjectParams Params;
    MemObject(const Params *params);

    const Params *
    params() const
    {
        return dynamic_cast<const Params *>(_params);
    }

  protected:
    // static: support for old-style constructors (call manually)
    static Params *makeParams(const std::string &name);

  public:
    /** Additional function to return the Port of a memory object. */
    virtual Port *getPort(const std::string &if_name, int idx = -1) = 0;

    /** Tell object that this port is about to disappear, so it should remove it
     * from any structures that it's keeping it in. */
    virtual void deletePortRefs(Port *p) ;
};

#endif //__MEM_MEM_OBJECT_HH__
