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
 * Authors: Steven K. Reinhardt
 */

#include "base/loader/raw_object.hh"
#include "base/loader/symtab.hh"
#include "base/trace.hh"

ObjectFile *
RawObject::tryFile(const std::string &fname, int fd, size_t len, uint8_t *data)
{
    return new RawObject(fname, fd, len, data, ObjectFile::UnknownArch,
            ObjectFile::UnknownOpSys);
}

RawObject::RawObject(const std::string &_filename, int _fd, size_t _len,
        uint8_t *_data, Arch _arch, OpSys _opSys)
    : ObjectFile(_filename, _fd, _len, _data, _arch, _opSys)
{
    text.baseAddr = 0;
    text.size = len;
    text.fileImage = fileData;

    data.baseAddr = 0;
    data.size = 0;
    data.fileImage = NULL;

    bss.baseAddr = 0;
    bss.size = 0;
    bss.fileImage = NULL;

    DPRINTFR(Loader, "text: 0x%x %d\ndata: 0x%x %d\nbss: 0x%x %d\n",
             text.baseAddr, text.size, data.baseAddr, data.size,
             bss.baseAddr, bss.size);
}

bool
RawObject::loadGlobalSymbols(SymbolTable *symtab, Addr addrMask)
{
/*    int fnameStart = filename.rfind('/',filename.size()) + 1;
    int extStart = filename.rfind('.',filename.size());
    symtab->insert(text.baseAddr & addrMask, filename.substr(fnameStart,
                extStart-fnameStart) + "_start");*/
    return true;
}

bool
RawObject::loadLocalSymbols(SymbolTable *symtab, Addr addrMask)
{
/*    int fnameStart = filename.rfind('/',filename.size()) + 1;
    int extStart = filename.rfind('.',filename.size());
    symtab->insert(text.baseAddr & addrMask, filename.substr(fnameStart,
                extStart-fnameStart) + "_start");*/
    return true;
}
