/*
 * Copyright (c) 2007 MIPS Technologies, Inc.  All Rights Reserved
 *
 * This software is part of the M5 simulator.
 *
 * THIS IS A LEGAL AGREEMENT.  BY DOWNLOADING, USING, COPYING, CREATING
 * DERIVATIVE WORKS, AND/OR DISTRIBUTING THIS SOFTWARE YOU ARE AGREEING
 * TO THESE TERMS AND CONDITIONS.
 *
 * Permission is granted to use, copy, create derivative works and
 * distribute this software and such derivative works for any purpose,
 * so long as (1) the copyright notice above, this grant of permission,
 * and the disclaimer below appear in all copies and derivative works
 * made, (2) the copyright notice above is augmented as appropriate to
 * reflect the addition of any new copyrightable work in a derivative
 * work (e.g., Copyright (c) <Publication Year> Copyright Owner), and (3)
 * the name of MIPS Technologies, Inc. (¡ÈMIPS¡É) is not used in any
 * advertising or publicity pertaining to the use or distribution of
 * this software without specific, written prior authorization.
 *
 * THIS SOFTWARE IS PROVIDED ¡ÈAS IS.¡É  MIPS MAKES NO WARRANTIES AND
 * DISCLAIMS ALL WARRANTIES, WHETHER EXPRESS, STATUTORY, IMPLIED OR
 * OTHERWISE, INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND
 * NON-INFRINGEMENT OF THIRD PARTY RIGHTS, REGARDING THIS SOFTWARE.
 * IN NO EVENT SHALL MIPS BE LIABLE FOR ANY DAMAGES, INCLUDING DIRECT,
 * INDIRECT, INCIDENTAL, CONSEQUENTIAL, SPECIAL, OR PUNITIVE DAMAGES OF
 * ANY KIND OR NATURE, ARISING OUT OF OR IN CONNECTION WITH THIS AGREEMENT,
 * THIS SOFTWARE AND/OR THE USE OF THIS SOFTWARE, WHETHER SUCH LIABILITY
 * IS ASSERTED ON THE BASIS OF CONTRACT, TORT (INCLUDING NEGLIGENCE OR
 * STRICT LIABILITY), OR OTHERWISE, EVEN IF MIPS HAS BEEN WARNED OF THE
 * POSSIBILITY OF ANY SUCH LOSS OR DAMAGE IN ADVANCE.
 *
 * Authors: Richard Strong 
 *
 */

/** @file
 * Implementation of Malta platform.
 */

#include <deque>
#include <string>
#include <vector>

#include "cpu/intr_control.hh"
#include "dev/simconsole.hh"
#include "dev/mips/malta_cchip.hh"
#include "dev/mips/malta_pchip.hh"
#include "dev/mips/malta_io.hh"
#include "dev/mips/malta.hh"
#include "params/Malta.hh"
#include "sim/system.hh"


using namespace std;
using namespace TheISA;

Malta::Malta(const Params *p)
    : Platform(p), system(p->system)
{
    // set the back pointer from the system to myself
    system->platform = this;

    for (int i = 0; i < Malta::Max_CPUs; i++)
        intr_sum_type[i] = 0;
}

Tick
Malta::intrFrequency()
{
    return io->frequency();
}

void
Malta::postConsoleInt()
{
     //panic("Malta::postConsoleInt() has not been implemented.");
    io->postIntr(0x10/*HW4*/);//see {Linux-src}/arch/mips/mips-boards/sim/sim_setup.c
}

void
Malta::clearConsoleInt()
{
        //FIXME: implement clearConsoleInt()
        //warn("Malta::clearConsoleInt() has not been implemented.");
    io->clearIntr(0x10/*HW4*/);
}

void
Malta::postPciInt(int line)
{
                panic("Malta::postPciInt() has not been implemented.");
    //cchip->postDRIR(line);
}

void
Malta::clearPciInt(int line)
{
                panic("Malta::clearPciInt() has not been implemented.");
    //cchip->clearDRIR(line);
}

Addr
Malta::pciToDma(Addr pciAddr) const
{
                panic("Malta::pciToDma() has not been implemented.");
    return pchip->translatePciToDma(pciAddr);
}


Addr
Malta::calcConfigAddr(int bus, int dev, int func)
{
        panic("Malta::calcConfigAddr() has not been implemented.");
   return pchip->calcConfigAddr(bus, dev, func);
}

void
Malta::serialize(std::ostream &os)
{

    SERIALIZE_ARRAY(intr_sum_type, Malta::Max_CPUs);
}

void
Malta::unserialize(Checkpoint *cp, const std::string &section)
{
    UNSERIALIZE_ARRAY(intr_sum_type, Malta::Max_CPUs);
}

Malta *
MaltaParams::create()
{
    return new Malta(this);
}
