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

/** @file
 * Implementation of T1000 platform.
 */

#include <deque>
#include <string>
#include <vector>

#include "cpu/intr_control.hh"
#include "dev/simconsole.hh"
#include "dev/sparc/t1000.hh"
#include "sim/system.hh"

using namespace std;
//Should this be AlphaISA?
using namespace TheISA;

T1000::T1000(const Params *p)
    : Platform(p), system(p->system)
{
    // set the back pointer from the system to myself
    system->platform = this;
}

Tick
T1000::intrFrequency()
{
    panic("Need implementation\n");
    M5_DUMMY_RETURN
}

void
T1000::postConsoleInt()
{
    warn_once("Don't know what interrupt to post for console.\n");
    //panic("Need implementation\n");
}

void
T1000::clearConsoleInt()
{
    warn_once("Don't know what interrupt to clear for console.\n");
    //panic("Need implementation\n");
}

void
T1000::postPciInt(int line)
{
    panic("Need implementation\n");
}

void
T1000::clearPciInt(int line)
{
    panic("Need implementation\n");
}

Addr
T1000::pciToDma(Addr pciAddr) const
{
    panic("Need implementation\n");
    M5_DUMMY_RETURN
}


Addr
T1000::calcConfigAddr(int bus, int dev, int func)
{
    panic("Need implementation\n");
    M5_DUMMY_RETURN
}

T1000 *
T1000Params::create()
{
    return new T1000(this);
}
