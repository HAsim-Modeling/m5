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
 *          Steven K. Reinhardt
 */

#ifndef __SYMTAB_HH__
#define __SYMTAB_HH__

#include <iosfwd>
#include <map>

#include "sim/host.hh"	// for Addr

class Checkpoint;
class SymbolTable
{
  public:
    typedef std::map<Addr, std::string> ATable;
    typedef std::map<std::string, Addr> STable;

  private:
    ATable addrTable;
    STable symbolTable;

  private:
    bool
    upperBound(Addr addr, ATable::const_iterator &iter) const
    {
        // find first key *larger* than desired address
        iter = addrTable.upper_bound(addr);

        // if very first key is larger, we're out of luck
        if (iter == addrTable.begin())
            return false;

        return true;
    }

  public:
    SymbolTable() {}
    SymbolTable(const std::string &file) { load(file); }
    ~SymbolTable() {}

    void clear();
    bool insert(Addr address, std::string symbol);
    bool load(const std::string &file);

    const ATable &getAddrTable() const { return addrTable; }
    const STable &getSymbolTable() const { return symbolTable; }

  public:
    void serialize(const std::string &base, std::ostream &os);
    void unserialize(const std::string &base, Checkpoint *cp,
                     const std::string &section);

  public:
    bool
    findSymbol(Addr address, std::string &symbol) const
    {
        ATable::const_iterator i = addrTable.find(address);
        if (i == addrTable.end())
            return false;

        symbol = (*i).second;
        return true;
    }

    bool
    findAddress(const std::string &symbol, Addr &address) const
    {
        STable::const_iterator i = symbolTable.find(symbol);
        if (i == symbolTable.end())
            return false;

        address = (*i).second;
        return true;
    }

    /// Find the nearest symbol equal to or less than the supplied
    /// address (e.g., the label for the enclosing function).
    /// @param addr     The address to look up.
    /// @param symbol   Return reference for symbol string.
    /// @param symaddr  Return reference for symbol address.
    /// @param nextaddr Address of following symbol (for
    ///                 determining valid range of symbol).
    /// @retval True if a symbol was found.
    bool
    findNearestSymbol(Addr addr, std::string &symbol, Addr &symaddr,
                      Addr &nextaddr) const
    {
        ATable::const_iterator i;
        if (!upperBound(addr, i))
            return false;

        nextaddr = i->first;
        --i;
        symaddr = i->first;
        symbol = i->second;
        return true;
    }

    /// Overload for findNearestSymbol() for callers who don't care
    /// about nextaddr.
    bool
    findNearestSymbol(Addr addr, std::string &symbol, Addr &symaddr) const
    {
        ATable::const_iterator i;
        if (!upperBound(addr, i))
            return false;

        --i;
        symaddr = i->first;
        symbol = i->second;
        return true;
    }


    bool
    findNearestAddr(Addr addr, Addr &symaddr, Addr &nextaddr) const
    {
        ATable::const_iterator i;
        if (!upperBound(addr, i))
            return false;

        nextaddr = i->first;
        --i;
        symaddr = i->first;
        return true;
    }

    bool
    findNearestAddr(Addr addr, Addr &symaddr) const
    {
        ATable::const_iterator i;
        if (!upperBound(addr, i))
            return false;

        --i;
        symaddr = i->first;
        return true;
    }
};

/// Global unified debugging symbol table (for target).  Conceptually
/// there should be one of these per System object for full system,
/// and per Process object for non-full-system, but so far one big
/// global one has worked well enough.
extern SymbolTable *debugSymbolTable;

#endif // __SYMTAB_HH__
