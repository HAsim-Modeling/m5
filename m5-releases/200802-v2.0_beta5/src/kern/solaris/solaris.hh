/*
 * Copyright (c) 2004, 2005
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

#ifndef __SOLARIS_HH__
#define __SOLARIS_HH__
#include "config/full_system.hh"

#if FULL_SYSTEM

class Solaris {};

#else //!FULL_SYSTEM

#include <inttypes.h>

#include "kern/operatingsystem.hh"

class TranslatingPort;

///
/// This class encapsulates the types, structures, constants,
/// functions, and syscall-number mappings specific to the Solaris
/// syscall interface.
///
class Solaris : public OperatingSystem
{

  public:

    //@{
    /// Basic Solaris types.
    typedef uint64_t size_t;
    typedef uint64_t off_t;
    typedef int64_t time_t;
    typedef int32_t uid_t;
    typedef int32_t gid_t;
    typedef uint64_t rlim_t;
    typedef uint64_t ino_t;
    typedef uint64_t dev_t;
    typedef uint32_t mode_t;
    typedef uint32_t nlink_t;
    //@}

    struct tgt_timespec {
        int64_t tv_sec;
        int64_t tv_nsec;
    };

    /// Stat buffer.  Note that we can't call it 'stat' since that
    /// gets #defined to something else on some systems.
    typedef struct {
        uint64_t	st_dev;		//!< device
        uint64_t	st_ino;		//!< inode
        uint32_t	st_mode;	//!< mode
        uint32_t	st_nlink;	//!< link count
        int32_t	        st_uid;		//!< owner's user ID
        int32_t	        st_gid;		//!< owner's group ID
        uint64_t	st_rdev;	//!< device number
        int64_t		st_size;	//!< file size in bytes
        //struct tgt_timespec	st_atimeX;	//!< time of last access
        //struct tgt_timespec	st_mtimeX;	//!< time of last modification
        //struct tgt_timespec	st_ctimeX;	//!< time of last status change
        int64_t st_atimeX, st_mtimeX, st_ctimeX;
        int32_t	        st_blksize;	//!< optimal I/O block size
        int64_t		st_blocks;	//!< number of blocks allocated
        char            st_fstype[16];
    } tgt_stat;

    // same for stat64
    typedef struct {
        uint64_t	st_dev;		//!< device
        uint64_t	st_ino;		//!< inode
        uint32_t	st_mode;	//!< mode
        uint32_t	st_nlink;	//!< link count
        int32_t	        st_uid;		//!< owner's user ID
        int32_t	        st_gid;		//!< owner's group ID
        uint64_t	st_rdev;	//!< device number
        int64_t		st_size;	//!< file size in bytes
        //struct tgt_timespec	st_atimeX;	//!< time of last access
        //struct tgt_timespec	st_mtimeX;	//!< time of last modification
        //struct tgt_timespec	st_ctimeX;	//!< time of last status change
        int64_t st_atimeX, st_mtimeX, st_ctimeX;
        int32_t	        st_blksize;	//!< optimal I/O block size
        int64_t		st_blocks;	//!< number of blocks allocated
        char            st_fstype[16];
    } tgt_stat64;

    /// Length of strings in struct utsname (plus 1 for null char).
    static const int _SYS_NMLN = 257;

    /// Interface struct for uname().
    typedef struct utsname {
        char sysname[_SYS_NMLN];	//!< System name.
        char nodename[_SYS_NMLN];	//!< Node name.
        char release[_SYS_NMLN];	//!< OS release.
        char version[_SYS_NMLN];	//!< OS version.
        char machine[_SYS_NMLN];	//!< Machine type.
    } utsname;

};  // class Solaris


#endif // FULL_SYSTEM

#endif // __SOLARIS_HH__
