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
 * Authors: Nathan L. Binkert
 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "base/loader/symtab.hh"
#include "base/misc.hh"
#include "base/str.hh"
#include "sim/host.hh"
#include "sim/serialize.hh"

using namespace std;

SymbolTable *debugSymbolTable = NULL;

void
SymbolTable::clear()
{
    addrTable.clear();
    symbolTable.clear();
}

bool
SymbolTable::insert(Addr address, string symbol)
{
    if (symbol.empty())
        return false;

    if (!addrTable.insert(make_pair(address, symbol)).second)
        return false;

    if (!symbolTable.insert(make_pair(symbol, address)).second)
        return false;

    return true;
}


bool
SymbolTable::load(const string &filename)
{
    string buffer;
    ifstream file(filename.c_str());

    if (!file)
        fatal("file error: Can't open symbol table file %s\n", filename);

    while (!file.eof()) {
        getline(file, buffer);
        if (buffer.empty())
            continue;

        int idx = buffer.find(',');
        if (idx == string::npos)
            return false;

        string address = buffer.substr(0, idx);
        eat_white(address);
        if (address.empty())
            return false;

        string symbol = buffer.substr(idx + 1);
        eat_white(symbol);
        if (symbol.empty())
            return false;

        Addr addr;
        if (!to_number(address, addr))
            return false;

        if (!insert(addr, symbol))
            return false;
    }

    file.close();

    return true;
}

void
SymbolTable::serialize(const string &base, ostream &os)
{
    paramOut(os, base + ".size", addrTable.size());

    int i = 0;
    ATable::const_iterator p, end = addrTable.end();
    for (p = addrTable.begin(); p != end; ++p) {
        paramOut(os, csprintf("%s.addr_%d", base, i), p->first);
        paramOut(os, csprintf("%s.symbol_%d", base, i), p->second);
        ++i;
    }
}

void
SymbolTable::unserialize(const string &base, Checkpoint *cp,
                         const string &section)
{
    clear();
    int size;
    paramIn(cp, section, base + ".size", size);
    for (int i = 0; i < size; ++i) {
        Addr addr;
        std::string symbol;

        paramIn(cp, section, csprintf("%s.addr_%d", base, i), addr);
        paramIn(cp, section, csprintf("%s.symbol_%d", base, i), symbol);
        insert(addr, symbol);
    }
}
