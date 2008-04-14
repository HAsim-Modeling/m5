/*
 * Copyright (c) 2001, 2002, 2003, 2004, 2005
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

#ifndef __PROCESS_HH__
#define __PROCESS_HH__

//
// The purpose of this code is to fake the loader & syscall mechanism
// when there's no OS: thus there's no reason to use it in FULL_SYSTEM
// mode when we do have an OS.
//
#include "config/full_system.hh"

#if !FULL_SYSTEM

#include <string>
#include <vector>

#include "base/statistics.hh"
#include "sim/host.hh"
#include "sim/sim_object.hh"

class GDBListener;
class PageTable;
class ProcessParams;
class LiveProcessParams;
class SyscallDesc;
class System;
class ThreadContext;
class TranslatingPort;
namespace TheISA
{
    class RemoteGDB;
}

class Process : public SimObject
{
  public:

    /// Pointer to object representing the system this process is
    /// running on.
    System *system;

    // have we initialized a thread context from this process?  If
    // yes, subsequent contexts are assumed to be for dynamically
    // created threads and are not initialized.
    bool initialContextLoaded;

    bool checkpointRestored;

    // thread contexts associated with this process
    std::vector<ThreadContext *> threadContexts;

    // remote gdb objects
    std::vector<TheISA::RemoteGDB *> remoteGDB;
    std::vector<GDBListener *> gdbListen;
    bool breakpoint();

    // number of CPUs (esxec contexts, really) assigned to this process.
    unsigned int numCpus() { return threadContexts.size(); }

    // record of blocked context
    struct WaitRec
    {
        Addr waitChan;
        ThreadContext *waitingContext;

        WaitRec(Addr chan, ThreadContext *ctx)
            : waitChan(chan), waitingContext(ctx)
        {	}
    };

    // list of all blocked contexts
    std::list<WaitRec> waitList;

    Addr brk_point;		// top of the data segment

    Addr stack_base;		// stack segment base (highest address)
    unsigned stack_size;	// initial stack size
    Addr stack_min;		// lowest address accessed on the stack

    // The maximum size allowed for the stack.
    Addr max_stack_size;

    // addr to use for next stack region (for multithreaded apps)
    Addr next_thread_stack_base;

    // Base of region for mmaps (when user doesn't specify an address).
    Addr mmap_start;
    Addr mmap_end;

    // Base of region for nxm data
    Addr nxm_start;
    Addr nxm_end;

    std::string prog_fname;	// file name

    Stats::Scalar<> num_syscalls;	// number of syscalls executed


  protected:
    // constructor
    Process(ProcessParams * params);

    // post initialization startup
    virtual void startup();

  protected:
    /// Memory object for initialization (image loading)
    TranslatingPort *initVirtMem;

  public:
    PageTable *pTable;

    //This id is assigned by m5 and is used to keep process' tlb entries
    //separated.
    uint64_t M5_pid;

    class FdMap
    {
      public:
            int fd;
            std::string filename;
            int mode;
            int flags;
            bool isPipe;
            int readPipeSource;
            uint64_t fileOffset;


            FdMap()
            {
                    fd = -1;
                    filename = "NULL";
                    mode = 0;
                    flags = 0;
                    isPipe = false;
                    readPipeSource = 0;
                    fileOffset = 0;

            }

        void serialize(std::ostream &os);
        void unserialize(Checkpoint *cp, const std::string &section);

    };

  private:
    // file descriptor remapping support
    static const int MAX_FD = 256;    // max legal fd value
    FdMap fd_map[MAX_FD+1];


  public:
    // static helper functions to generate file descriptors for constructor
    static int openInputFile(const std::string &filename);
    static int openOutputFile(const std::string &filename);

    // override of virtual SimObject method: register statistics
    virtual void regStats();

    // register a thread context for this process.
    // returns tc's cpu number (index into threadContexts[])
    int registerThreadContext(ThreadContext *tc);


    void replaceThreadContext(ThreadContext *tc, int tcIndex);

    // map simulator fd sim_fd to target fd tgt_fd
    void dup_fd(int sim_fd, int tgt_fd);

    // generate new target fd for sim_fd
    int alloc_fd(int sim_fd, std::string filename, int flags, int mode, bool pipe);

    // free target fd (e.g., after close)
    void free_fd(int tgt_fd);

    // look up simulator fd for given target fd
    int sim_fd(int tgt_fd);

    // look up simulator fd_map object for a given target fd
    FdMap * sim_fd_obj(int tgt_fd);

    // fix all offsets for currently open files and save them
    void fix_file_offsets();

    // find all offsets for currently open files and save them
    void find_file_offsets();

    // set the source of this read pipe for a checkpoint resume
    void setReadPipeSource(int read_pipe_fd, int source_fd);

    virtual void syscall(int64_t callnum, ThreadContext *tc) = 0;

    // check if the this addr is on the next available page and allocate it
    // if it's not we'll panic
    bool checkAndAllocNextPage(Addr vaddr);

    void serialize(std::ostream &os);
    void unserialize(Checkpoint *cp, const std::string &section);
};

//
// "Live" process with system calls redirected to host system
//
class ObjectFile;
class LiveProcess : public Process
{
  protected:
    ObjectFile *objFile;
    std::vector<std::string> argv;
    std::vector<std::string> envp;
    std::string cwd;

    LiveProcess(LiveProcessParams * params, ObjectFile *objFile);

    virtual void argsInit(int intSize, int pageSize);

    // Id of the owner of the process
    uint64_t __uid;
    uint64_t __euid;
    uint64_t __gid;
    uint64_t __egid;

    // pid of the process and it's parent
    uint64_t __pid;
    uint64_t __ppid;

  public:

    enum AuxiliaryVectorType {
        M5_AT_NULL = 0,
        M5_AT_IGNORE = 1,
        M5_AT_EXECFD = 2,
        M5_AT_PHDR = 3,
        M5_AT_PHENT = 4,
        M5_AT_PHNUM = 5,
        M5_AT_PAGESZ = 6,
        M5_AT_BASE = 7,
        M5_AT_FLAGS = 8,
        M5_AT_ENTRY = 9,
        M5_AT_NOTELF = 10,
        M5_AT_UID = 11,
        M5_AT_EUID = 12,
        M5_AT_GID = 13,
        M5_AT_EGID = 14,
        // The following may be specific to Linux
        M5_AT_PLATFORM = 15,
        M5_AT_HWCAP = 16,
        M5_AT_CLKTCK = 17,

        M5_AT_SECURE = 23,

        M5_AT_VECTOR_SIZE = 44
    };

    inline uint64_t uid() {return __uid;}
    inline uint64_t euid() {return __euid;}
    inline uint64_t gid() {return __gid;}
    inline uint64_t egid() {return __egid;}
    inline uint64_t pid() {return __pid;}
    inline uint64_t ppid() {return __ppid;}

    std::string
    fullPath(const std::string &filename)
    {
        if (filename[0] == '/' || cwd.empty())
            return filename;

        std::string full = cwd;

        if (cwd[cwd.size() - 1] != '/')
            full += '/';

        return full + filename;
    }

    virtual void syscall(int64_t callnum, ThreadContext *tc);

    virtual SyscallDesc* getDesc(int callnum) = 0;

    // this function is used to create the LiveProcess object, since
    // we can't tell which subclass of LiveProcess to use until we
    // open and look at the object file.
    static LiveProcess *create(LiveProcessParams * params);
};


#endif // !FULL_SYSTEM

#endif // __PROCESS_HH__
