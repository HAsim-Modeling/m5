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
#include <string>
#include <vector>

#include "base/loader/object_file.hh"
#include "base/loader/symtab.hh"
#include "base/misc.hh"
#include "base/str.hh"

using namespace std;
Tick curTick;

ostream *outputStream = &cout;

int
main(int argc, char *argv[])
{
    if (argc != 2 && argc != 3)
        panic("usage: %s <filename> <symbol>\n", argv[0]);

    ObjectFile *obj = createObjectFile(argv[1]);
    if (!obj)
        panic("file not found\n");

    SymbolTable symtab;
    obj->loadGlobalSymbols(&symtab);
    obj->loadLocalSymbols(&symtab);

    if (argc == 2) {
        SymbolTable::ATable::const_iterator i = symtab.getAddrTable().begin();
        SymbolTable::ATable::const_iterator end = symtab.getAddrTable().end();
        while (i != end) {
            cprintf("%#x %s\n", i->first, i->second);
            ++i;
        }
    } else {
        string symbol = argv[2];
        Addr address;

        if (symbol[0] == '0' && symbol[1] == 'x') {
            if (to_number(symbol, address) &&
                symtab.findSymbol(address, symbol))
                cprintf("address = %#x, symbol = %s\n", address, symbol);
            else
                cprintf("address = %#x was not found\n", address);
        } else {
            if (symtab.findAddress(symbol, address))
                cprintf("symbol = %s address = %#x\n", symbol, address);
            else
                cprintf("symbol = %s was not found\n", symbol);
        }
    }

    return 0;
}
