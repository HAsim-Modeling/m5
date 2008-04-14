/*
 * Copyright (c) 2006
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
 * Authors: Korey L. Sewell
 *
 */

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
 * Authors: Korey Sewell
 *          Jaidev Patwardhan
 *
 */

#ifndef __ARCH_MIPS_REGFILE_MISC_REGFILE_HH__
#define __ARCH_MIPS_REGFILE_MISC_REGFILE_HH__

#include "arch/mips/isa_traits.hh"
#include "arch/mips/types.hh"
#include "arch/mips/mt.hh"
#include "arch/mips/mt_constants.hh"
#include "base/bitfield.hh"
#include "sim/eventq.hh"
#include <queue>

class Params;
class BaseCPU;

namespace MipsISA
{
    class MiscRegFile {
      public:
        // Give RegFile object, private access
        friend class RegFile;

	// The MIPS name for this file is CP0 or Coprocessor 0
        typedef MiscRegFile CP0;

      protected:
	enum BankType {
	    perProcessor,
	    perThreadContext,
	    perVirtProcessor
	};

	std::vector<std::vector<MiscReg> > miscRegFile;
	std::vector<std::vector<MiscReg> > miscRegFile_WriteMask;
	std::vector<BankType> bankType;

	BaseCPU *cpu;
       
      public:
	MiscRegFile();
	MiscRegFile(BaseCPU *cpu);

	void init();

        void clear(unsigned tid_or_vpn = 0);

        void reset(std::string core_name, unsigned num_threads, unsigned num_vpes, BaseCPU *_cpu);

	void expandForMultithreading(unsigned num_threads, unsigned num_vpes);

	inline unsigned getVPENum(unsigned tid);

	//////////////////////////////////////////////////////////
	//
	// READ/WRITE CP0 STATE
	// 
	//
	//////////////////////////////////////////////////////////
	//@TODO: MIPS MT's register view automatically connects 
	//       Status to TCStatus depending on current thread
	void updateCP0ReadView(int misc_reg, unsigned tid) { }
	MiscReg readRegNoEffect(int misc_reg, unsigned tid = 0);

        //template <class TC>
	MiscReg readReg(int misc_reg, 
			ThreadContext *tc,  unsigned tid = 0);

        MiscReg filterCP0Write(int misc_reg, int reg_sel, const MiscReg &val);
        void setRegMask(int misc_reg, const MiscReg &val, unsigned tid = 0);
	void setRegNoEffect(int misc_reg, const MiscReg &val, unsigned tid = 0);

        //template <class TC>
	void setReg(int misc_reg, const MiscReg &val, 
		     ThreadContext *tc, unsigned tid = 0);

        int getInstAsid();
        int getDataAsid();
	
	//////////////////////////////////////////////////////////
	//
	// DECLARE INTERFACE THAT WILL ALLOW A MiscRegFile (Cop0)
	// TO SCHEDULE EVENTS
	//
	//////////////////////////////////////////////////////////

	// Flag that is set when CP0 state has been written to.
	bool cp0Updated;

	// Enumerated List of CP0 Event Types
	enum CP0EventType {
	    UpdateCP0
	};
	
	// Declare A CP0Event Class for scheduling
	class CP0Event : public Event
	{
	  protected:
	    MiscRegFile::CP0 *cp0;
	    BaseCPU *cpu;
	    CP0EventType cp0EventType;
	    Fault fault;

	  public:
	    /** Constructs a CP0 event. */
	    CP0Event(CP0 *_cp0, BaseCPU *_cpu, CP0EventType e_type);

	    /** Process this event. */
	    virtual void process();

	    /** Returns the description of this event. */
            const char *description() const;

	    /** Schedule This Event */
	    void scheduleEvent(int delay);

	    /** Unschedule This Event */
	    void unscheduleEvent();
	};

	// Schedule a CP0 Update Event
	void scheduleCP0Update(int delay = 0);

	// If any changes have been made, then check the state for changes
	// and if necessary alert the CPU
	void updateCPU();

	// Keep a List of CPU Events that need to be deallocated
	std::queue<CP0Event*> cp0EventRemoveList;

        static std::string miscRegNames[NumMiscRegs];	
    };

    inline std::string getMiscRegName(unsigned reg_idx);
} // namespace MipsISA

#endif
