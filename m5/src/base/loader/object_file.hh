/*
 * Copyright (c) 2002, 2003, 2004
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

#ifndef __OBJECT_FILE_HH__
#define __OBJECT_FILE_HH__

#include <limits>
#include <string>

#include "sim/host.hh"	// for Addr

class Port;
class SymbolTable;

class ObjectFile
{
  public:

    enum Arch {
        UnknownArch,
        Alpha,
        SPARC64,
        SPARC32,
        Mips,
        Arm
    };

    enum OpSys {
        UnknownOpSys,
        Tru64,
        Linux,
        Solaris
    };

  protected:
    const std::string filename;
    int descriptor;
    uint8_t *fileData;
    size_t len;

    Arch  arch;
    OpSys opSys;

    ObjectFile(const std::string &_filename, int _fd,
               size_t _len, uint8_t *_data,
               Arch _arch, OpSys _opSys);

  public:
    virtual ~ObjectFile();

    void close();

    virtual bool loadSections(Port *memPort, Addr addrMask =
            std::numeric_limits<Addr>::max());
    virtual bool loadGlobalSymbols(SymbolTable *symtab, Addr addrMask =
            std::numeric_limits<Addr>::max()) = 0;
    virtual bool loadLocalSymbols(SymbolTable *symtab, Addr addrMask =
            std::numeric_limits<Addr>::max()) = 0;

    virtual bool isDynamic() { return false; }
    virtual bool hasTLS() { return false; }

    Arch  getArch()  const { return arch; }
    OpSys getOpSys() const { return opSys; }

  protected:

    struct Section {
        Addr     baseAddr;
        uint8_t *fileImage;
        size_t   size;
    };

    Addr entry;
    Addr globalPtr;

    Section text;
    Section data;
    Section bss;

    bool loadSection(Section *sec, Port *memPort, Addr addrMask);
    void setGlobalPointer(Addr global_ptr) { globalPtr = global_ptr; }

  public:
    Addr entryPoint() const { return entry; }

    Addr globalPointer() const { return globalPtr; }

    Addr textBase() const { return text.baseAddr; }
    Addr dataBase() const { return data.baseAddr; }
    Addr bssBase() const { return bss.baseAddr; }

    size_t textSize() const { return text.size; }
    size_t dataSize() const { return data.size; }
    size_t bssSize() const { return bss.size; }

    void setTextBase(Addr a) { text.baseAddr = a; }
};

ObjectFile *createObjectFile(const std::string &fname, bool raw = false);


#endif // __OBJECT_FILE_HH__
