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

#ifndef __ARCH_SPARC_LINUX_LINUX_HH__
#define __ARCH_SPARC_LINUX_LINUX_HH__

#include "kern/linux/linux.hh"

class SparcLinux : public Linux
{
  public:

    typedef struct {
        uint32_t st_dev;
        char __pad1[4];
        uint64_t st_ino;
        uint32_t st_mode;
        uint16_t st_nlink;
        uint32_t st_uid;
        uint32_t st_gid;
        uint32_t st_rdev;
        char __pad2[4];
        int64_t st_size;
        int64_t st_atimeX;
        int64_t st_mtimeX;
        int64_t st_ctimeX;
        int64_t st_blksize;
        int64_t st_blocks;
        uint64_t __unused4[2];
    } tgt_stat;

    static OpenFlagTransTable openFlagTable[];

    static const int TGT_O_RDONLY	= 0x00000000;	//!< O_RDONLY
    static const int TGT_O_WRONLY	= 0x00000001;	//!< O_WRONLY
    static const int TGT_O_RDWR	        = 0x00000002;	//!< O_RDWR
    static const int TGT_O_NONBLOCK     = 0x00004000;	//!< O_NONBLOCK
    static const int TGT_O_APPEND	= 0x00000008;	//!< O_APPEND
    static const int TGT_O_CREAT	= 0x00000200;	//!< O_CREAT
    static const int TGT_O_TRUNC	= 0x00000400;	//!< O_TRUNC
    static const int TGT_O_EXCL	        = 0x00000800;	//!< O_EXCL
    static const int TGT_O_NOCTTY	= 0x00008000;	//!< O_NOCTTY
    static const int TGT_O_SYNC	        = 0x00002000;	//!< O_SYNC
//    static const int TGT_O_DRD	        = 0x00010000;	//!< O_DRD
//    static const int TGT_O_DIRECTIO     = 0x00020000;	//!< O_DIRECTIO
//    static const int TGT_O_CACHE	= 0x00002000;	//!< O_CACHE
//    static const int TGT_O_DSYNC	= 0x00008000;	//!< O_DSYNC
//    static const int TGT_O_RSYNC	= 0x00040000;	//!< O_RSYNC

    static const int NUM_OPEN_FLAGS;

    static const unsigned TGT_MAP_ANONYMOUS = 0x20;
};

class Sparc32Linux : public SparcLinux
{
  public:

    typedef struct {
        uint64_t st_dev;
        uint64_t st_ino;
        uint32_t st_mode;
        uint32_t st_nlink;
        uint32_t st_uid;
        uint32_t st_gid;
        uint64_t st_rdev;
        uint8_t __pad3[8];
        int64_t st_size;
        int32_t st_blksize;
        uint8_t __pad4[8];
        int64_t st_blocks;
        uint64_t st_atimeX;
        uint64_t st_atime_nsec;
        uint64_t st_mtimeX;
        uint64_t st_mtime_nsec;
        uint64_t st_ctimeX;
        uint64_t st_ctime_nsec;
        uint32_t __unused4;
        uint32_t __unused5;
    } tgt_stat64;
};

#endif
