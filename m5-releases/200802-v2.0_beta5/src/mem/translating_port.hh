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
 * Authors: Ronald G. Dreslinski Jr
 *          Ali G. Saidi
 */

#ifndef __MEM_TRANSLATING_PROT_HH__
#define __MEM_TRANSLATING_PROT_HH__

#include "mem/port.hh"

class PageTable;
class Process;

class TranslatingPort : public FunctionalPort
{
  public:
    enum AllocType {
        Always,
        Never,
        NextPage
    };

  private:
    PageTable *pTable;
    Process *process;
    AllocType allocating;

  public:
    TranslatingPort(const std::string &_name,
                    Process *p, AllocType alloc);
    virtual ~TranslatingPort();

    bool tryReadBlob(Addr addr, uint8_t *p, int size);
    bool tryWriteBlob(Addr addr, uint8_t *p, int size);
    bool tryMemsetBlob(Addr addr, uint8_t val, int size);
    bool tryWriteString(Addr addr, const char *str);
    bool tryReadString(std::string &str, Addr addr);

    virtual void readBlob(Addr addr, uint8_t *p, int size);
    virtual void writeBlob(Addr addr, uint8_t *p, int size);
    virtual void memsetBlob(Addr addr, uint8_t val, int size);

    void writeString(Addr addr, const char *str);
    void readString(std::string &str, Addr addr);
};

#endif
