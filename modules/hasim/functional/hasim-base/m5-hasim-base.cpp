//
// Copyright (C) 2008 Intel Corporation
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

//
// @file m5_hasim_base.cpp
// @brief Low level interface to m5 simulator
// @author Michael Adler
//

#include <signal.h>

#include "asim/syntax.h"
#include "asim/mesg.h"

#include "asim/provides/command_switches.h"
#include "asim/provides/m5_hasim_base.h"

// m5
#include "sim/m5_main.hh"
#include "sim/sim_object.hh"
extern SimObject *resolveSimObject(const string &);

#include <Python.h>

ATOMIC32_CLASS M5_HASIM_BASE_CLASS::refCnt;
AtomicSimpleCPU_PTR *M5_HASIM_BASE_CLASS::m5cpus;
UINT32 M5_HASIM_BASE_CLASS::numCPUs;


void
m5HAsimExitNowHandler(int sigtype)
{
    exit(1);
}


M5_HASIM_BASE_CLASS::M5_HASIM_BASE_CLASS()
{
    if (refCnt++ == 0)
    {
        //
        // Initialize m5
        //
        // M5 expects the executable name to be in argv[0]
        // and to receive MAX_NUM_CPUS in --num-cpus
        char** new_argv = new char* [globalArgs->FuncPlatformArgc() + 3];
        new_argv[0] = globalArgs->ExecutableName();


        for (int i = 0; i < globalArgs->FuncPlatformArgc(); i++)
        {
            new_argv[i + 1] = strdup(globalArgs->FuncPlatformArgv()[i]);
        }

        char* cpuArg = new char[32];
        numCPUs = MAX_NUM_CONTEXTS;
        sprintf(cpuArg, "--num-cpus=%d", numCPUs);
        new_argv[globalArgs->FuncPlatformArgc() + 1] = cpuArg;

        m5_main(globalArgs->FuncPlatformArgc() + 2, new_argv);

        // Drop m5 handling of SIGINT and SIGABRT.  These don't work well since
        // m5's event loop isn't running.  Simply exit, hoping that some buffers
        // will be flushed.
        signal(SIGINT, m5HAsimExitNowHandler);
        signal(SIGABRT, m5HAsimExitNowHandler);

        ASSERTX(numCPUs != 0);

        // Cache pointers to m5 CPUs here
        m5cpus = new AtomicSimpleCPU_PTR[numCPUs];

        if (numCPUs == 1)
        {
            // If there is a single CPU it is named system.cpu
            SimObject *so = resolveSimObject("system.cpu");
            m5cpus[0] = dynamic_cast<AtomicSimpleCPU*>(so);
            ASSERT(m5cpus[0] != NULL, "Failed to find m5 cpu object:  system.cpu");
        }
        else
        {
            // If there is a single CPU it is named system.cpu
            for (int cpu = 0; cpu < numCPUs; cpu++)
            {
                char cpu_name[64];
                sprintf(cpu_name, "system.cpu%d", cpu);
                SimObject *so = resolveSimObject(cpu_name);
                m5cpus[cpu] = dynamic_cast<AtomicSimpleCPU*>(so);
                ASSERT(m5cpus[cpu] != NULL, "Failed to find m5 cpu object: " << cpu_name);
            }
        }
    }
}


M5_HASIM_BASE_CLASS::~M5_HASIM_BASE_CLASS()
{
    if (refCnt-- == 1)
    {
        Py_Finalize();
        delete[] m5cpus;
    }
}
