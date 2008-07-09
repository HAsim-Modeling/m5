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
        m5_main(globalArgs->FuncPlatformArgc(), globalArgs->FuncPlatformArgv());

        // Drop m5 handling of SIGINT and SIGABRT.  These don't work well since
        // m5's event loop isn't running.  Simply exit, hoping that some buffers
        // will be flushed.
        signal(SIGINT, m5HAsimExitNowHandler);
        signal(SIGABRT, m5HAsimExitNowHandler);
    }

    SimObject *so = resolveSimObject("system.cpu");
    m5cpu = dynamic_cast<AtomicSimpleCPU*>(so);
    ASSERT(m5cpu != NULL, "Failed to find m5 cpu object");
}


M5_HASIM_BASE_CLASS::~M5_HASIM_BASE_CLASS()
{
    if (refCnt-- == 1)
    {
        Py_Finalize();
    }
}
