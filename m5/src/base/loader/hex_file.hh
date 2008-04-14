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
 * Authors: Jaidev Patwardhan
 *
 */

#ifndef __HEX_FILE_HH__
#define __HEX_FILE_HH__

#include <limits>
#include <string>

#include "sim/host.hh"	// for Addr
#include <fstream>

class Port;

class HexFile
{
  public:


  protected:
    const std::string filename;
    FILE *fp;

  public:
    virtual ~HexFile();
    HexFile(const std::string _filename);

    void close();

    bool loadSections(Port *memPort, Addr addrMask =
            std::numeric_limits<Addr>::max());

  protected:

  typedef struct { 
    Addr MemAddr;
    uint32_t Data;
  } HexLine;

    Addr entry;
    Addr globalPtr;

  public:
    void parseLine(char *,Addr *,uint32_t *);
    Addr entryPoint() const { return entry; }
    Addr globalPointer() const { return globalPtr; }

};

#endif // __HEX_FILE_HH__
