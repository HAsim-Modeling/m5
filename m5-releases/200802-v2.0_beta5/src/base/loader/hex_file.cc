/*
 * Copyright (c) 2007 MIPS Technologies, Inc.  All Rights Reserved
 *
 * This software is part of the M5 simulator.
 *
 * THIS IS A LEGAL AGREEMENT.  BY DOWNLOADING, USING, COPYING, CREATING
 * DERIVATIVE WORKS, AND/OR DISTRIBUTING THIS SOFTWARE YOU ARE AGREEING
 * TO THESE TERMS AND CONDITIONS.
 *
 * Permission is granted to use, copy, create derivative works and
 * distribute this software and such derivative works for any purpose,
 * so long as (1) the copyright notice above, this grant of permission,
 * and the disclaimer below appear in all copies and derivative works
 * made, (2) the copyright notice above is augmented as appropriate to
 * reflect the addition of any new copyrightable work in a derivative
 * work (e.g., Copyright (c) <Publication Year> Copyright Owner), and (3)
 * the name of MIPS Technologies, Inc. (¡ÈMIPS¡É) is not used in any
 * advertising or publicity pertaining to the use or distribution of
 * this software without specific, written prior authorization.
 *
 * THIS SOFTWARE IS PROVIDED ¡ÈAS IS.¡É  MIPS MAKES NO WARRANTIES AND
 * DISCLAIMS ALL WARRANTIES, WHETHER EXPRESS, STATUTORY, IMPLIED OR
 * OTHERWISE, INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND
 * NON-INFRINGEMENT OF THIRD PARTY RIGHTS, REGARDING THIS SOFTWARE.
 * IN NO EVENT SHALL MIPS BE LIABLE FOR ANY DAMAGES, INCLUDING DIRECT,
 * INDIRECT, INCIDENTAL, CONSEQUENTIAL, SPECIAL, OR PUNITIVE DAMAGES OF
 * ANY KIND OR NATURE, ARISING OUT OF OR IN CONNECTION WITH THIS AGREEMENT,
 * THIS SOFTWARE AND/OR THE USE OF THIS SOFTWARE, WHETHER SUCH LIABILITY
 * IS ASSERTED ON THE BASIS OF CONTRACT, TORT (INCLUDING NEGLIGENCE OR
 * STRICT LIABILITY), OR OTHERWISE, EVEN IF MIPS HAS BEEN WARNED OF THE
 * POSSIBILITY OF ANY SUCH LOSS OR DAMAGE IN ADVANCE.
 *
 * Authors: Jaidev Patwardhan
 */

#include <list>
#include <string>

#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "base/cprintf.hh"
#include "base/loader/hex_file.hh"
#include "base/loader/symtab.hh"


#include "mem/translating_port.hh"

using namespace std;
/* Load a Hex File into memory.
   Currently only used with MIPS BARE_IRON mode.
   A hex file consists of [Address Data] tuples that get directly loaded into
   physical memory. The address specified is a word address (i.e., to get the byte address, shift left by 2)
   The data is a full 32-bit hex value.
*/
HexFile::HexFile(const string _filename)
    : filename(_filename)
{
  fp = fopen(filename.c_str(),"r");
  if(fp == NULL)
    {
      panic("Unable to open %s\n",filename.c_str());
    }
    
}

HexFile::~HexFile()
{
}


bool
HexFile::loadSections(Port *memPort, Addr addrMask)
{
  char Line[64];
  Addr MemAddr;
  uint32_t Data;
  while(!feof(fp))
    {
      fgets(Line,64,fp);
      parseLine(Line,&MemAddr,&Data);
      //      printf("Hex:%u\n",Data);
      
      if(MemAddr != 0)
	{
	  // Now, write to memory
	  memPort->writeBlob(MemAddr<<2,(uint8_t *)&Data,sizeof(Data));
	}
    }
    return true;
}
void HexFile::parseLine(char *Str,Addr *A, uint32_t *D)
{
  int i=0;
  bool Flag = false;
  *A = 0;
  *D = 0;
  int Digit = 0;
  unsigned Number = 0;
  /* Skip white spaces */
  while(Str[i] != '\0' && Str[i]==' ')
    i++;

  /* Ok, we're at some character...process things */
  while(Str[i] != '\0')
    {
      if(Str[i]>='0' && Str[i]<='9')
	{
	  Digit=Str[i]-'0';
	}
      else if(Str[i]>='a' && Str[i]<='f')
	{
	  Digit=Str[i]-'a'+10;
	}
      else if(Str[i]>='A' && Str[i]<='F')
	{
	  Digit=Str[i]-'A'+10;
	}
      else if(Str[i] == ' ' || Str[i]=='\n')
	{
	  if(Number == 0)
	    return;
	  if(Flag == false)
	    {
	      *A = Number;
	      Number = 0;
	      Flag = true;
	    }
	  else
	    {
	      *D = Number;
	      return;
	    }
	}
      else
	{
	  // Ok, we've encountered a non-hex character, cannot be a valid line, skip and return 0's
	  *A = 0;
	  *D = 0;
	  return;
	}
      Number<<=4;
      Number+=Digit;
      i++;
      
    }
  if(Flag != true)
    {
      *A = 0;
      *D = 0;
    }
  else
    *D = Number;
  
}



void
HexFile::close()
{
  fclose(fp);
}
