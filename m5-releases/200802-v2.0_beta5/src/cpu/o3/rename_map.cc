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
 * Authors: Kevin T. Lim
 */

#include <vector>

#include "cpu/o3/rename_map.hh"

using namespace std;

// @todo: Consider making inline bool functions that determine if the
// register is a logical int, logical fp, physical int, physical fp,
// etc.

SimpleRenameMap::~SimpleRenameMap()
{
}

void
SimpleRenameMap::init(unsigned _numLogicalIntRegs,
                      unsigned _numPhysicalIntRegs,
                      PhysRegIndex &ireg_idx,

                      unsigned _numLogicalFloatRegs,
                      unsigned _numPhysicalFloatRegs,
                      PhysRegIndex &freg_idx,

                      unsigned _numMiscRegs,

                      RegIndex _intZeroReg,
                      RegIndex _floatZeroReg,

                      int map_id,
                      bool bindRegs)
{
    id = map_id;

    numLogicalIntRegs = _numLogicalIntRegs;

    numLogicalFloatRegs = _numLogicalFloatRegs;

    numPhysicalIntRegs = _numPhysicalIntRegs;

    numPhysicalFloatRegs = _numPhysicalFloatRegs;

    numMiscRegs = _numMiscRegs;

    intZeroReg = _intZeroReg;
    floatZeroReg = _floatZeroReg;

    DPRINTF(Rename, "Creating rename map %i.  Phys: %i / %i, Float: "
            "%i / %i.\n", id, numLogicalIntRegs, numPhysicalIntRegs,
            numLogicalFloatRegs, numPhysicalFloatRegs);

    numLogicalRegs = numLogicalIntRegs + numLogicalFloatRegs;

    numPhysicalRegs = numPhysicalIntRegs + numPhysicalFloatRegs;

    //Create the rename maps
    intRenameMap.resize(numLogicalIntRegs);
    floatRenameMap.resize(numLogicalRegs);

    if (bindRegs) {
        DPRINTF(Rename, "Binding registers into rename map %i",id);

        // Initialize the entries in the integer rename map to point to the
        // physical registers of the same index
        for (RegIndex index = 0; index < numLogicalIntRegs; ++index)
        {
            intRenameMap[index].physical_reg = ireg_idx++;
        }

        // Initialize the entries in the floating point rename map to point to
        // the physical registers of the same index
        // Although the index refers purely to architected registers, because
        // the floating reg indices come after the integer reg indices, they
        // may exceed the size of a normal RegIndex (short).
        for (PhysRegIndex index = numLogicalIntRegs;
             index < numLogicalRegs; ++index)
        {
            floatRenameMap[index].physical_reg = freg_idx++;
        }
    } else {
        DPRINTF(Rename, "Binding registers into rename map %i",id);

        PhysRegIndex temp_ireg = ireg_idx;

        for (RegIndex index = 0; index < numLogicalIntRegs; ++index)
        {
            intRenameMap[index].physical_reg = temp_ireg++;
        }

        PhysRegIndex temp_freg = freg_idx;

        for (PhysRegIndex index = numLogicalIntRegs;
             index < numLogicalRegs; ++index)
        {
            floatRenameMap[index].physical_reg = temp_freg++;
        }
    }
}

void
SimpleRenameMap::setFreeList(SimpleFreeList *fl_ptr)
{
    freeList = fl_ptr;
}


SimpleRenameMap::RenameInfo
SimpleRenameMap::rename(RegIndex arch_reg)
{
    PhysRegIndex renamed_reg;
    PhysRegIndex prev_reg;

    if (arch_reg < numLogicalIntRegs) {

        // Record the current physical register that is renamed to the
        // requested architected register.
        prev_reg = intRenameMap[arch_reg].physical_reg;

        // If it's not referencing the zero register, then rename the
        // register.
        if (arch_reg != intZeroReg) {
            renamed_reg = freeList->getIntReg();

            intRenameMap[arch_reg].physical_reg = renamed_reg;

            assert(renamed_reg >= 0 && renamed_reg < numPhysicalIntRegs);

        } else {
            // Otherwise return the zero register so nothing bad happens.
            renamed_reg = intZeroReg;
        }
    } else if (arch_reg < numLogicalRegs) {
        // Record the current physical register that is renamed to the
        // requested architected register.
        prev_reg = floatRenameMap[arch_reg].physical_reg;

        // If it's not referencing the zero register, then rename the
        // register.
#if THE_ISA == ALPHA_ISA
        if (arch_reg != floatZeroReg) {
#endif
            renamed_reg = freeList->getFloatReg();

            floatRenameMap[arch_reg].physical_reg = renamed_reg;

            assert(renamed_reg < numPhysicalRegs &&
                   renamed_reg >= numPhysicalIntRegs);
#if THE_ISA == ALPHA_ISA
        } else {
            // Otherwise return the zero register so nothing bad happens.
            renamed_reg = floatZeroReg;
        }
#endif
    } else {
        // Subtract off the base offset for miscellaneous registers.
        arch_reg = arch_reg - numLogicalRegs;

        DPRINTF(Rename, "Renamed misc reg %d\n", arch_reg);

        // No renaming happens to the misc. registers.  They are
        // simply the registers that come after all the physical
        // registers; thus take the base architected register and add
        // the physical registers to it.
        renamed_reg = arch_reg + numPhysicalRegs;

        // Set the previous register to the same register; mainly it must be
        // known that the prev reg was outside the range of normal registers
        // so the free list can avoid adding it.
        prev_reg = renamed_reg;
    }

    DPRINTF(Rename, "Renamed reg %d to physical reg %d old mapping was %d\n",
            arch_reg, renamed_reg, prev_reg);

    return RenameInfo(renamed_reg, prev_reg);
}

PhysRegIndex
SimpleRenameMap::lookup(RegIndex arch_reg)
{
    if (arch_reg < numLogicalIntRegs) {
        return intRenameMap[arch_reg].physical_reg;
    } else if (arch_reg < numLogicalRegs) {
        return floatRenameMap[arch_reg].physical_reg;
    } else {
        // Subtract off the misc registers offset.
        arch_reg = arch_reg - numLogicalRegs;

        // Misc. regs don't rename, so simply add the base arch reg to
        // the number of physical registers.
        return numPhysicalRegs + arch_reg;
    }
}

void
SimpleRenameMap::setEntry(RegIndex arch_reg, PhysRegIndex renamed_reg)
{
    // In this implementation the miscellaneous registers do not
    // actually rename, so this function does not allow you to try to
    // change their mappings.
    if (arch_reg < numLogicalIntRegs) {
        DPRINTF(Rename, "Rename Map: Integer register %i being set to %i.\n",
                (int)arch_reg, renamed_reg);

        intRenameMap[arch_reg].physical_reg = renamed_reg;
    } else if (arch_reg < numLogicalIntRegs + numLogicalFloatRegs) {
        DPRINTF(Rename, "Rename Map: Float register %i being set to %i.\n",
                (int)arch_reg - numLogicalIntRegs, renamed_reg);

        floatRenameMap[arch_reg].physical_reg = renamed_reg;
    }
}

int
SimpleRenameMap::numFreeEntries()
{
    int free_int_regs = freeList->numFreeIntRegs();
    int free_float_regs = freeList->numFreeFloatRegs();

    if (free_int_regs < free_float_regs) {
        return free_int_regs;
    } else {
        return free_float_regs;
    }
}
