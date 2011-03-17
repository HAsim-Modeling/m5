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

#ifndef __LINUX_HH__
#define __LINUX_HH__
#include "config/full_system.hh"

#if FULL_SYSTEM

class Linux {};

#else //!FULL_SYSTEM

#include <inttypes.h>

#include "kern/operatingsystem.hh"

///
/// This class encapsulates the types, structures, constants,
/// functions, and syscall-number mappings specific to the Alpha Linux
/// syscall interface.
///
class Linux : public OperatingSystem
{

  public:

    //@{
    /// Basic Linux types.
    typedef uint64_t size_t;
    typedef uint64_t off_t;
    typedef int64_t time_t;
    typedef uint32_t uid_t;
    typedef uint32_t gid_t;
    //@}

    /// Stat buffer.  Note that we can't call it 'stat' since that
    /// gets #defined to something else on some systems. This type
    /// can be specialized by architecture specific "Linux" classes
    typedef struct {
        uint32_t	st_dev;		//!< device
        uint32_t	st_ino;		//!< inode
        uint32_t	st_mode;	//!< mode
        uint32_t	st_nlink;	//!< link count
        uint32_t	st_uid;		//!< owner's user ID
        uint32_t	st_gid;		//!< owner's group ID
        uint32_t	st_rdev;	//!< device number
        int32_t		_pad1;		//!< for alignment
        int64_t		st_size;	//!< file size in bytes
        uint64_t	st_atimeX;	//!< time of last access
        uint64_t	st_mtimeX;	//!< time of last modification
        uint64_t	st_ctimeX;	//!< time of last status change
        uint32_t	st_blksize;	//!< optimal I/O block size
        int32_t		st_blocks;	//!< number of blocks allocated
        uint32_t	st_flags;	//!< flags
        uint32_t	st_gen;		//!< unknown
    } tgt_stat;

    // same for stat64
    typedef struct {
        uint64_t	st_dev;
        uint64_t	st_ino;
        uint64_t	st_rdev;
        int64_t		st_size;
        uint64_t	st_blocks;

        uint32_t	st_mode;
        uint32_t	st_uid;
        uint32_t	st_gid;
        uint32_t	st_blksize;
        uint32_t	st_nlink;
        uint32_t	__pad0;

        uint64_t	st_atimeX;
        uint64_t 	st_atime_nsec;
        uint64_t	st_mtimeX;
        uint64_t	st_mtime_nsec;
        uint64_t	st_ctimeX;
        uint64_t	st_ctime_nsec;
        int64_t		___unused[3];
    } tgt_stat64;

    /// Length of strings in struct utsname (plus 1 for null char).
    static const int _SYS_NMLN = 65;

    /// Interface struct for uname().
    struct utsname {
        char sysname[_SYS_NMLN];	//!< System name.
        char nodename[_SYS_NMLN];	//!< Node name.
        char release[_SYS_NMLN];	//!< OS release.
        char version[_SYS_NMLN];	//!< OS version.
        char machine[_SYS_NMLN];	//!< Machine type.
    };

    /// Limit struct for getrlimit/setrlimit.
    struct rlimit {
        uint64_t  rlim_cur;	//!< soft limit
        uint64_t  rlim_max;	//!< hard limit
    };

    /// For gettimeofday().
    struct timeval {
        int64_t tv_sec;		//!< seconds
        int64_t tv_usec;	//!< microseconds
    };

    // For writev/readv
    struct tgt_iovec {
        uint64_t iov_base; // void *
        uint64_t iov_len;
    };


    /// For getrusage().
    struct rusage {
        struct timeval ru_utime;	//!< user time used
        struct timeval ru_stime;	//!< system time used
        int64_t ru_maxrss;		//!< max rss
        int64_t ru_ixrss;		//!< integral shared memory size
        int64_t ru_idrss;		//!< integral unshared data "
        int64_t ru_isrss;		//!< integral unshared stack "
        int64_t ru_minflt;		//!< page reclaims - total vmfaults
        int64_t ru_majflt;		//!< page faults
        int64_t ru_nswap;		//!< swaps
        int64_t ru_inblock;		//!< block input operations
        int64_t ru_oublock;		//!< block output operations
        int64_t ru_msgsnd;		//!< messages sent
        int64_t ru_msgrcv;		//!< messages received
        int64_t ru_nsignals;		//!< signals received
        int64_t ru_nvcsw;		//!< voluntary context switches
        int64_t ru_nivcsw;		//!< involuntary "
    };

};  // class Linux


#endif // FULL_SYSTEM

#endif // __LINUX_HH__
