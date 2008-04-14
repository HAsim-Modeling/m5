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
 * Authors: Gabe M. Black
 */

#include "arch/sparc/linux/linux.hh"
#include <fcntl.h>

// open(2) flags translation table
OpenFlagTransTable SparcLinux::openFlagTable[] = {
#ifdef _MSC_VER
  { SparcLinux::TGT_O_RDONLY,	_O_RDONLY },
  { SparcLinux::TGT_O_WRONLY,	_O_WRONLY },
  { SparcLinux::TGT_O_RDWR,	_O_RDWR },
  { SparcLinux::TGT_O_APPEND,	_O_APPEND },
  { SparcLinux::TGT_O_CREAT,	_O_CREAT },
  { SparcLinux::TGT_O_TRUNC,	_O_TRUNC },
  { SparcLinux::TGT_O_EXCL,	_O_EXCL },
#ifdef _O_NONBLOCK
  { SparcLinux::TGT_O_NONBLOCK,	_O_NONBLOCK },
#endif
#ifdef _O_NOCTTY
  { SparcLinux::TGT_O_NOCTTY,	_O_NOCTTY },
#endif
#ifdef _O_SYNC
  { SparcLinux::TGT_O_SYNC,	_O_SYNC },
#endif
#else /* !_MSC_VER */
  { SparcLinux::TGT_O_RDONLY,	O_RDONLY },
  { SparcLinux::TGT_O_WRONLY,	O_WRONLY },
  { SparcLinux::TGT_O_RDWR,	O_RDWR },
  { SparcLinux::TGT_O_APPEND,	O_APPEND },
  { SparcLinux::TGT_O_CREAT,	O_CREAT },
  { SparcLinux::TGT_O_TRUNC,	O_TRUNC },
  { SparcLinux::TGT_O_EXCL,	O_EXCL },
  { SparcLinux::TGT_O_NONBLOCK,	O_NONBLOCK },
  { SparcLinux::TGT_O_NOCTTY,	O_NOCTTY },
#ifdef O_SYNC
  { SparcLinux::TGT_O_SYNC,	O_SYNC },
#endif
#endif /* _MSC_VER */
};

const int SparcLinux::NUM_OPEN_FLAGS =
        (sizeof(SparcLinux::openFlagTable)/sizeof(SparcLinux::openFlagTable[0]));

