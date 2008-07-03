//
// INTEL CONFIDENTIAL
// Copyright (c) 2008 Intel Corp.  Recipient is granted a non-sublicensable 
// copyright license under Intel copyrights to copy and distribute this code 
// internally only. This code is provided "AS IS" with no support and with no 
// warranties of any kind, including warranties of MERCHANTABILITY,
// FITNESS FOR ANY PARTICULAR PURPOSE or INTELLECTUAL PROPERTY INFRINGEMENT. 
// By making any use of this code, Recipient agrees that no other licenses 
// to any Intel patents, trade secrets, copyrights or other intellectual 
// property rights are granted herein, and no other licenses shall arise by 
// estoppel, implication or by operation of law. Recipient accepts all risks 
// of use.
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
