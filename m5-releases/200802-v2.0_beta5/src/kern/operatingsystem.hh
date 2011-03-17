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
 * Authors: Gabe M. Black
 */

#ifndef __KERN_OPERATINGSYSTEM_HH__
#define __KERN_OPERATINGSYSTEM_HH__

#include "config/full_system.hh"

#include <inttypes.h>

#if FULL_SYSTEM

class OperatingSystem {};

#else //!FULL_SYSTEM

/// This struct is used to build an target-OS-dependent table that
/// maps the target's open() flags to the host open() flags.
struct OpenFlagTransTable {
    int tgtFlag;	//!< Target system flag value.
    int hostFlag;	//!< Corresponding host system flag value.
};


///
/// This class encapsulates the types, structures, constants,
/// functions, and syscall-number mappings specific to an operating system
/// syscall interface.
///
class OperatingSystem {

  public:

    /// Stat buffer.  Note that we can't call it 'stat' since that
    /// gets #defined to something else on some systems. This type
    /// can be specialized by architecture specific "Linux" classes
    typedef void tgt_stat;

    // same for stat64
    typedef void tgt_stat64;

    /// Length of strings in struct utsname (plus 1 for null char).
    static const int _SYS_NMLN = 65;

    /// Interface struct for uname().
    typedef struct {
        char sysname[_SYS_NMLN];	//!< System name.
        char nodename[_SYS_NMLN];	//!< Node name.
        char release[_SYS_NMLN];	//!< OS release.
        char version[_SYS_NMLN];	//!< OS version.
        char machine[_SYS_NMLN];	//!< Machine type.
    } utsname;

    /// Limit struct for getrlimit/setrlimit.
    typedef struct {
        uint64_t  rlim_cur;	//!< soft limit
        uint64_t  rlim_max;	//!< hard limit
    } rlimit;

    /// For gettimeofday().
    typedef struct {
        int64_t tv_sec;		//!< seconds
        int64_t tv_usec;	//!< microseconds
    } timeval;

    // For writev/readv
    typedef struct {
        uint64_t iov_base; // void *
        uint64_t iov_len;
    } tgt_iovec;


    /// For getrusage().
    typedef struct {
        timeval ru_utime;	//!< user time used
        timeval ru_stime;	//!< system time used
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
    } rusage;

};  // class OperatingSystem


#endif // FULL_SYSTEM

#endif // __OPERATINGSYSTEM_HH__
