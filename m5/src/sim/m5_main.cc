/*
 * Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005
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
 */

#include <Python.h>
#include <signal.h>

#include <iostream>
#include <string>

#include "base/cprintf.hh"
#include "base/misc.hh"
#include "config/pythonhome.hh"
#include "python/swig/init.hh"
#include "sim/async.hh"
#include "sim/host.hh"
#include "sim/core.hh"

using namespace std;

/// Stats signal handler.
void
dumpStatsHandler(int sigtype)
{
    async_event = true;
    async_statdump = true;
}

void
dumprstStatsHandler(int sigtype)
{
    async_event = true;
    async_statdump = true;
    async_statreset = true;
}

/// Exit signal handler.
void
exitNowHandler(int sigtype)
{
    async_event = true;
    async_exit = true;
}

/// Abort signal handler.
void
abortHandler(int sigtype)
{
    ccprintf(cerr, "Program aborted at cycle %d\n", curTick);
}

int
python_main()
{
    PyObject *module;
    PyObject *dict;
    PyObject *result;

    module = PyImport_AddModule(const_cast<char*>("__main__"));
    if (module == NULL)
        fatal("Could not import __main__");

    dict = PyModule_GetDict(module);

    result = PyRun_String("import m5.main", Py_file_input, dict, dict);
    if (!result) {
        PyErr_Print();
        return 1;
    }
    Py_DECREF(result);

    result = PyRun_String("m5.main.main()", Py_file_input, dict, dict);
    if (!result) {
        PyErr_Print();
        return 1;
    }
    Py_DECREF(result);

    if (Py_FlushLine())
        PyErr_Clear();

    return 0;
}

int
m5_main(int argc, char **argv)
{
    signal(SIGFPE, SIG_IGN);		// may occur on misspeculated paths
    signal(SIGTRAP, SIG_IGN);
    signal(SIGUSR1, dumpStatsHandler);		// dump intermediate stats
    signal(SIGUSR2, dumprstStatsHandler);	// dump and reset stats
    signal(SIGINT, exitNowHandler);		// dump final stats and exit
    signal(SIGABRT, abortHandler);

    Py_SetProgramName(argv[0]);

    // default path to m5 python code is the currently executing
    // file... Python ZipImporter will find embedded zip archive.
    // The M5_ARCHIVE environment variable can be used to override this.
    char *m5_archive = getenv("M5_ARCHIVE");
    string pythonpath = m5_archive ? m5_archive : argv[0];

    char *oldpath = getenv("PYTHONPATH");
    if (oldpath != NULL) {
        pythonpath += ":";
        pythonpath += oldpath;
    }

    if (setenv("PYTHONPATH", pythonpath.c_str(), true) == -1)
        fatal("setenv: %s\n", strerror(errno));

    const char *python_home = getenv("PYTHONHOME");
    if (!python_home)
        python_home = PYTHONHOME;
    Py_SetPythonHome(const_cast<char*>(python_home));

    // initialize embedded Python interpreter
    Py_Initialize();
    PySys_SetArgv(argc, argv);

    // initialize SWIG modules
    init_swig();

    return python_main();
}
