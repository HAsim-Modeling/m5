/*
 * Copyright (c) 2003, 2004, 2005
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
 *          Korey L. Sewell
 */

#include "arch/mips/regfile/regfile.hh"
#include "sim/serialize.hh"

using namespace std;

namespace MipsISA 
{

void 
RegFile::clear()
{
    intRegFile.clear();
    floatRegFile.clear();
    miscRegFile.clear();
}

void  
RegFile::reset(std::string core_name, unsigned num_threads, unsigned num_vpes, BaseCPU *_cpu)
{
    bzero(&intRegFile, sizeof(intRegFile));
    bzero(&floatRegFile, sizeof(floatRegFile));
    miscRegFile.reset(core_name, num_threads, num_vpes, _cpu);
}

IntReg  
RegFile::readIntReg(int intReg)
{
    return intRegFile.readReg(intReg);
}

Fault  
RegFile::setIntReg(int intReg, const IntReg &val)
{
    return intRegFile.setReg(intReg, val);
}

MiscReg  
RegFile::readMiscRegNoEffect(int miscReg, unsigned tid)
{
    return miscRegFile.readRegNoEffect(miscReg, tid);
}

MiscReg  
RegFile::readMiscReg(int miscReg, ThreadContext *tc,
		    unsigned tid)
{
    return miscRegFile.readReg(miscReg, tc, tid);
}

void  
RegFile::setMiscRegNoEffect(int miscReg, const MiscReg &val, unsigned tid)
{
    miscRegFile.setRegNoEffect(miscReg, val, tid);
}

void  
RegFile::setMiscReg(int miscReg, const MiscReg &val,
                ThreadContext * tc, unsigned tid)
{
    miscRegFile.setReg(miscReg, val, tc, tid);
}

FloatRegVal  
RegFile::readFloatReg(int floatReg)
{
    return floatRegFile.readReg(floatReg,SingleWidth);
}

FloatRegVal  
RegFile::readFloatReg(int floatReg, int width)
{
    return floatRegFile.readReg(floatReg,width);
}

FloatRegBits  
RegFile::readFloatRegBits(int floatReg)
{
    return floatRegFile.readRegBits(floatReg,SingleWidth);
}

FloatRegBits  
RegFile::readFloatRegBits(int floatReg, int width)
{
    return floatRegFile.readRegBits(floatReg,width);
}

Fault  
RegFile::setFloatReg(int floatReg, const FloatRegVal &val)
{
    return floatRegFile.setReg(floatReg, val, SingleWidth);
}

Fault  
RegFile::setFloatReg(int floatReg, const FloatRegVal &val, int width)
{
    return floatRegFile.setReg(floatReg, val, width);
}

Fault  
RegFile::setFloatRegBits(int floatReg, const FloatRegBits &val)
{
    return floatRegFile.setRegBits(floatReg, val, SingleWidth);
}

Fault  
RegFile::setFloatRegBits(int floatReg, const FloatRegBits &val, int width)
{
    return floatRegFile.setRegBits(floatReg, val, width);
}

void 
RegFile::setShadowSet(int css){
    intRegFile.setShadowSet(css);
}

int 
RegFile::instAsid()
{
    return miscRegFile.getInstAsid();
}

int 
RegFile::dataAsid()
{
    return miscRegFile.getDataAsid();
}

Addr 
RegFile::readPC()
{
    return pc;
}

void 
RegFile::setPC(Addr val)
{
    pc = val;
}

Addr 
RegFile::readNextPC()
{
    return npc;
}

void 
RegFile::setNextPC(Addr val)
{
    npc = val;
}

Addr 
RegFile::readNextNPC()
{
    return nnpc;
}

void 
RegFile::setNextNPC(Addr val)
{
    nnpc = val;
}

void
RegFile::serialize(std::ostream &os)
{
    intRegFile.serialize(os);
    //SERIALIZE_ARRAY(floatRegFile, NumFloatRegs);
    //SERIALZE_ARRAY(miscRegFile);
    //SERIALIZE_SCALAR(miscRegs.fpcr);
    //SERIALIZE_SCALAR(miscRegs.lock_flag);
    //SERIALIZE_SCALAR(miscRegs.lock_addr);
    SERIALIZE_SCALAR(pc);
    SERIALIZE_SCALAR(npc);
    SERIALIZE_SCALAR(nnpc);
}

void
RegFile::unserialize(Checkpoint *cp, const std::string &section)
{
    intRegFile.unserialize(cp, section);
    //UNSERIALIZE_ARRAY(floatRegFile);
    //UNSERIALZE_ARRAY(miscRegFile);
    //UNSERIALIZE_SCALAR(miscRegs.fpcr);
    //UNSERIALIZE_SCALAR(miscRegs.lock_flag);
    //UNSERIALIZE_SCALAR(miscRegs.lock_addr);
    UNSERIALIZE_SCALAR(pc);
    UNSERIALIZE_SCALAR(npc);
    UNSERIALIZE_SCALAR(nnpc);

}

} // namespace MipsISA
