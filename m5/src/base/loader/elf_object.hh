/*
 * Copyright (c) 2003, 2004, 2005
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
 */

#ifndef __ELF_OBJECT_HH__
#define __ELF_OBJECT_HH__

#include "base/loader/object_file.hh"
#include <set>
#include <vector>

class ElfObject : public ObjectFile
{
  protected:

    //The global definition of a "Section" is closest to elf's segments.
    typedef ObjectFile::Section Segment;

    //These values are provided to a linux process by the kernel, so we
    //need to keep them around.
    Addr _programHeaderTable;
    uint16_t _programHeaderSize;
    uint16_t _programHeaderCount;
    std::set<std::string> sectionNames;

    /// Helper functions for loadGlobalSymbols() and loadLocalSymbols().
    bool loadSomeSymbols(SymbolTable *symtab, int binding);

    ElfObject(const std::string &_filename, int _fd,
              size_t _len, uint8_t *_data,
              Arch _arch, OpSys _opSys);

    void getSections();
    bool sectionExists(std::string sec);

    std::vector<Segment> extraSegments;

  public:
    virtual ~ElfObject() {}

    bool loadSections(Port *memPort,
            Addr addrMask = std::numeric_limits<Addr>::max());
    virtual bool loadGlobalSymbols(SymbolTable *symtab, Addr addrMask =
            std::numeric_limits<Addr>::max());
    virtual bool loadLocalSymbols(SymbolTable *symtab, Addr addrMask =
            std::numeric_limits<Addr>::max());

    virtual bool isDynamic() { return sectionExists(".interp"); }
    virtual bool hasTLS() { return sectionExists(".tbss"); }

    static ObjectFile *tryFile(const std::string &fname, int fd,
                               size_t len, uint8_t *data);
    Addr programHeaderTable() {return _programHeaderTable;}
    uint16_t programHeaderSize() {return _programHeaderSize;}
    uint16_t programHeaderCount() {return _programHeaderCount;}
};

#endif // __ELF_OBJECT_HH__
