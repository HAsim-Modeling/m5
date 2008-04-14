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
 * Authors: Ali G. Saidi
 */

#ifndef __ARCH_SPARC_SOLARIS_SOLARIS_HH__
#define __ARCH_SPARC_SOLARIS_SOLARIS_HH__

#include "kern/solaris/solaris.hh"

class SparcSolaris : public Solaris
{
  public:

    static OpenFlagTransTable openFlagTable[];

    static const int TGT_O_RDONLY	= 0x00000000;	//!< O_RDONLY
    static const int TGT_O_WRONLY	= 0x00000001;	//!< O_WRONLY
    static const int TGT_O_RDWR	        = 0x00000002;	//!< O_RDWR
    static const int TGT_O_NDELAY       = 0x00000004;	//!< O_NONBLOCK
    static const int TGT_O_APPEND	= 0x00000008;	//!< O_APPEND
    static const int TGT_O_SYNC         = 0x00000010;   //!< O_SYNC
    static const int TGT_O_DSYNC        = 0x00000040;   //!< O_SYNC
    static const int TGT_O_RSYNC        = 0x00008000;   //!< O_SYNC
    static const int TGT_O_NONBLOCK     = 0x00000080;   //!< O_NONBLOCK
    static const int TGT_O_PRIV         = 0x00001000;   //??
    static const int TGT_O_LARGEFILE    = 0x00002000;   //??
    static const int TGT_O_CREAT	= 0x00000100;	//!< O_CREAT
    static const int TGT_O_TRUNC	= 0x00000200;	//!< O_TRUNC
    static const int TGT_O_EXCL	        = 0x00000400;	//!< O_EXCL
    static const int TGT_O_NOCTTY	= 0x00000800;	//!< O_NOCTTY
    static const int TGT_O_XATTR        = 0x00004000;	//??

    static const int NUM_OPEN_FLAGS;

    static const unsigned TGT_MAP_ANONYMOUS = 0x100;
};

#endif
