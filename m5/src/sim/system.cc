/*
 * Copyright (c) 2003, 2004, 2005, 2006
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
 * Authors: Steven K. Reinhardt
 *          Lisa R. Hsu
 *          Nathan L. Binkert
 *          Ali G. Saidi
 */

#include "arch/isa_traits.hh"
#include "arch/remote_gdb.hh"
#include "arch/utility.hh"
#include "base/loader/object_file.hh"
#include "base/loader/symtab.hh"
#include "base/trace.hh"
#include "cpu/thread_context.hh"
#include "mem/mem_object.hh"
#include "mem/physical.hh"
#include "sim/byteswap.hh"
#include "sim/system.hh"
#include "sim/debug.hh"
#if FULL_SYSTEM
#include "arch/vtophys.hh"
#include "kern/kernel_stats.hh"
#else
#include "params/System.hh"
#endif

using namespace std;
using namespace TheISA;

vector<System *> System::systemList;

int System::numSystemsRunning = 0;

System::System(Params *p)
    : SimObject(p), physmem(p->physmem), numcpus(0),
#if FULL_SYSTEM
      init_param(p->init_param),
      functionalPort(p->name + "-fport"),
      virtPort(p->name + "-vport"),
#else
      page_ptr(0),
      next_PID(0),
#endif
      memoryMode(p->mem_mode), _params(p)
{
    // add self to global system list
    systemList.push_back(this);

#if FULL_SYSTEM
    kernelSymtab = new SymbolTable;
    if (!debugSymbolTable)
        debugSymbolTable = new SymbolTable;


    /**
     * Get a functional port to memory
     */
    Port *mem_port;
    mem_port = physmem->getPort("functional");
    functionalPort.setPeer(mem_port);
    mem_port->setPeer(&functionalPort);

    mem_port = physmem->getPort("functional");
    virtPort.setPeer(mem_port);
    mem_port->setPeer(&virtPort);


    /**
     * Load the kernel code into memory
     */
    if (params()->kernel == "") {
        warn("No kernel set for full system simulation. Assuming you know what"
                " you're doing...\n");
    } else {
        // Load kernel code
        kernel = createObjectFile(params()->kernel);
	warn("kernel located at: %s", params()->kernel);

        if (kernel == NULL)
            fatal("Could not load kernel file %s", params()->kernel);

        // Load program sections into memory
        kernel->loadSections(&functionalPort, LoadAddrMask);

        // setup entry points
        kernelStart = kernel->textBase();
        kernelEnd = kernel->bssBase() + kernel->bssSize();
        kernelEntry = kernel->entryPoint();

        // load symbols
        if (!kernel->loadGlobalSymbols(kernelSymtab))
            panic("could not load kernel symbols\n");

        if (!kernel->loadLocalSymbols(kernelSymtab))
            panic("could not load kernel local symbols\n");

        if (!kernel->loadGlobalSymbols(debugSymbolTable))
            panic("could not load kernel symbols\n");

        if (!kernel->loadLocalSymbols(debugSymbolTable))
            panic("could not load kernel local symbols\n");

        DPRINTF(Loader, "Kernel start = %#x\n", kernelStart);
        DPRINTF(Loader, "Kernel end   = %#x\n", kernelEnd);
        DPRINTF(Loader, "Kernel entry = %#x\n", kernelEntry);
        DPRINTF(Loader, "Kernel loaded...\n");
    }
#endif // FULL_SYSTEM

    // increment the number of running systms
    numSystemsRunning++;
}

System::~System()
{
#if FULL_SYSTEM
    delete kernelSymtab;
    delete kernel;
#else
    panic("System::fixFuncEventAddr needs to be rewritten "
          "to work with syscall emulation");
#endif // FULL_SYSTEM}
}

int rgdb_wait = -1;
int rgdb_enable = true;

void
System::setMemoryMode(Enums::MemoryMode mode)
{
    assert(getState() == Drained);
    memoryMode = mode;
}

bool System::breakpoint()
{
    if (remoteGDB.size())
        return remoteGDB[0]->breakpoint();
    return false;
}

int
System::registerThreadContext(ThreadContext *tc, int id)
{
    if (id == -1) {
        for (id = 0; id < threadContexts.size(); id++) {
            if (!threadContexts[id])
                break;
        }
    }

    if (threadContexts.size() <= id)
        threadContexts.resize(id + 1);

    if (threadContexts[id])
        panic("Cannot have two CPUs with the same id (%d)\n", id);

    threadContexts[id] = tc;
    numcpus++;

    int port = getRemoteGDBPort();
    if (rgdb_enable && port) {
        RemoteGDB *rgdb = new RemoteGDB(this, tc);
        GDBListener *gdbl = new GDBListener(rgdb, port + id);
        gdbl->listen();
        /**
         * Uncommenting this line waits for a remote debugger to
         * connect to the simulator before continuing.
         */
        if (rgdb_wait != -1 && rgdb_wait == id)
            gdbl->accept();

        if (remoteGDB.size() <= id) {
            remoteGDB.resize(id + 1);
        }

        remoteGDB[id] = rgdb;
    }

    return id;
}

void
System::startup()
{
    int i;
    for (i = 0; i < threadContexts.size(); i++)
        TheISA::startupCPU(threadContexts[i], i);
}

void
System::replaceThreadContext(ThreadContext *tc, int id)
{
    if (id >= threadContexts.size()) {
        panic("replaceThreadContext: bad id, %d >= %d\n",
              id, threadContexts.size());
    }

    threadContexts[id] = tc;
    if (id < remoteGDB.size())
        remoteGDB[id]->replaceThreadContext(tc);
}

#if !FULL_SYSTEM
Addr
System::new_page()
{
    Addr return_addr = page_ptr << LogVMPageSize;
    ++page_ptr;
    if (return_addr >= physmem->size())
        fatal("Out of memory, please increase size of physical memory.");
    return return_addr;
}
#endif

void
System::serialize(ostream &os)
{
#if FULL_SYSTEM
    kernelSymtab->serialize("kernel_symtab", os);
#else // !FULL_SYSTEM
    SERIALIZE_SCALAR(page_ptr);
#endif
}


void
System::unserialize(Checkpoint *cp, const string &section)
{
#if FULL_SYSTEM
    kernelSymtab->unserialize("kernel_symtab", cp, section);
#else // !FULL_SYSTEM
    UNSERIALIZE_SCALAR(page_ptr);
#endif
}

void
System::printSystems()
{
    vector<System *>::iterator i = systemList.begin();
    vector<System *>::iterator end = systemList.end();
    for (; i != end; ++i) {
        System *sys = *i;
        cerr << "System " << sys->name() << ": " << hex << sys << endl;
    }
}

void
printSystems()
{
    System::printSystems();
}

const char *System::MemoryModeStrings[3] = {"invalid", "atomic",
    "timing"};

#if !FULL_SYSTEM

System *
SystemParams::create()
{
    System::Params *p = new System::Params;
    p->name = name;
    p->physmem = physmem;
    p->mem_mode = mem_mode;
    return new System(p);
}

#endif
