/*
 * Copyright (c) 2002, 2003, 2004, 2005
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

#error "Cannot include this file"

/**
 * The ExecContext is not a usable class.  It is simply here for
 * documentation purposes.  It shows the interface that is used by the
 * ISA to access and change CPU state.
 */
class ExecContext {
    // The register accessor methods provide the index of the
    // instruction's operand (e.g., 0 or 1), not the architectural
    // register index, to simplify the implementation of register
    // renaming.  We find the architectural register index by indexing
    // into the instruction's own operand index table.  Note that a
    // raw pointer to the StaticInst is provided instead of a
    // ref-counted StaticInstPtr to reduce overhead.  This is fine as
    // long as these methods don't copy the pointer into any long-term
    // storage (which is pretty hard to imagine they would have reason
    // to do).

    /** Reads an integer register. */
    uint64_t readIntRegOperand(const StaticInst *si, int idx);

    /** Reads a floating point register of a specific width. */
    FloatReg readFloatRegOperand(const StaticInst *si, int idx, int width);

    /** Reads a floating point register of single register width. */
    FloatReg readFloatRegOperand(const StaticInst *si, int idx);

    /** Reads a floating point register of a specific width in its
     * binary format, instead of by value. */
    FloatRegBits readFloatRegOperandBits(const StaticInst *si, int idx,
                                         int width);

    /** Reads a floating point register in its binary format, instead
     * of by value. */
    FloatRegBits readFloatRegOperandBits(const StaticInst *si, int idx);

    /** Sets an integer register to a value. */
    void setIntRegOperand(const StaticInst *si, int idx, uint64_t val);

    /** Sets a floating point register of a specific width to a value. */
    void setFloatRegOperand(const StaticInst *si, int idx, FloatReg val,
                            int width);

    /** Sets a floating point register of single width to a value. */
    void setFloatRegOperand(const StaticInst *si, int idx, FloatReg val);

    /** Sets the bits of a floating point register of a specific width
     * to a binary value. */
    void setFloatRegOperandBits(const StaticInst *si, int idx,
                                FloatRegBits val, int width);

    /** Sets the bits of a floating point register of single width
     * to a binary value. */
    void setFloatRegOperandBits(const StaticInst *si, int idx,
                                FloatRegBits val);

    /** Reads the PC. */
    uint64_t readPC();
    /** Reads the NextPC. */
    uint64_t readNextPC();
    /** Reads the Next-NextPC. Only for architectures like SPARC or MIPS. */
    uint64_t readNextNPC();

    /** Sets the PC. */
    void setPC(uint64_t val);
    /** Sets the NextPC. */
    void setNextPC(uint64_t val);
    /** Sets the Next-NextPC.  Only for architectures like SPARC or MIPS. */
    void setNextNPC(uint64_t val);

    /** Reads a miscellaneous register. */
    MiscReg readMiscRegNoEffect(int misc_reg);

    /** Reads a miscellaneous register, handling any architectural
     * side effects due to reading that register. */
    MiscReg readMiscReg(int misc_reg);

    /** Sets a miscellaneous register. */
    void setMiscRegNoEffect(int misc_reg, const MiscReg &val);

    /** Sets a miscellaneous register, handling any architectural
     * side effects due to writing that register. */
    void setMiscReg(int misc_reg, const MiscReg &val);

    /** Records the effective address of the instruction.  Only valid
     * for memory ops. */
    void setEA(Addr EA);
    /** Returns the effective address of the instruction.  Only valid
     * for memory ops. */
    Addr getEA();

    /** Returns a pointer to the ThreadContext. */
    ThreadContext *tcBase();

    /** Reads an address, creating a memory request with the given
     * flags.  Stores result of read in data. */
    template <class T>
    Fault read(Addr addr, T &data, unsigned flags);

    /** Writes to an address, creating a memory request with the given
     * flags.  Writes data to memory.  For store conditionals, returns
     * the result of the store in res. */
    template <class T>
    Fault write(T data, Addr addr, unsigned flags, uint64_t *res);

    /** Prefetches an address, creating a memory request with the
     * given flags. */
    void prefetch(Addr addr, unsigned flags);

    /** Hints to the memory system that an address will be written to
     * soon, with the given size.  Creates a memory request with the
     * given flags. */
    void writeHint(Addr addr, int size, unsigned flags);

#if FULL_SYSTEM
    /** Somewhat Alpha-specific function that handles returning from
     * an error or interrupt. */
    Fault hwrei();

    /**
     * Check for special simulator handling of specific PAL calls.  If
     * return value is false, actual PAL call will be suppressed.
     */
    bool simPalCheck(int palFunc);
#else
    /** Executes a syscall specified by the callnum. */
    void syscall(int64_t callnum);
#endif
};
