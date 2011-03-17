#ifndef TRACECHILD_SPARC_HH
#define TRACECHILD_SPARC_HH

#include <asm-sparc64/reg.h>
#include <assert.h>
#include <ostream>
#include <stdint.h>
#include <string>
#include <sys/ptrace.h>
#include <sys/types.h>

#include "tracechild.hh"

struct regs;

class SparcTraceChild : public TraceChild
{
public:
        enum RegNum
        {
                //Global registers
                G0, G1, G2, G3, G4, G5, G6, G7,
                //Output registers
                O0, O1, O2, O3, O4, O5, O6, O7,
                //Local registers
                L0, L1, L2, L3, L4, L5, L6, L7,
                //Input registers
                I0, I1, I2, I3, I4, I5, I6, I7,
                //Floating point
                F0, F2, F4, F6, F8, F10, F12, F14,
                F16, F18, F20, F22, F24, F26, F28, F30,
                F32, F34, F36, F38, F40, F42, F44, F46,
                F48, F50, F52, F54, F56, F58, F60, F62,
                //Miscelaneous
                FSR, FPRS, PC, NPC, Y, CWP, PSTATE, ASI, CCR,
                numregs
        };
private:
        char printBuffer[256];
        static std::string regNames[numregs];
        regs theregs;
        regs oldregs;
        fpu thefpregs;
        fpu oldfpregs;
        uint64_t locals[8];
        uint64_t oldLocals[8];
        uint64_t inputs[8];
        uint64_t oldInputs[8];
        bool regDiffSinceUpdate[numregs];

        //This calculates where the pc might go after the current instruction.
        //while this equals npc for most instructions, it doesn't for all of
        //them. The return value is the number of actual potential targets.
        int getTargets(uint32_t inst, uint64_t pc, uint64_t npc,
                uint64_t &target1, uint64_t &target2);

protected:
        bool update(int pid);

public:
        SparcTraceChild();

        bool sendState(int socket);

        int getNumRegs()
        {
                return numregs;
        }

        bool diffSinceUpdate(int num)
        {
                assert(num < numregs && num >= 0);
                return regDiffSinceUpdate[num];
        }

        std::string getRegName(int num)
        {
                assert(num < numregs && num >= 0);
                return regNames[num];
        }

        int64_t getRegVal(int num);

        int64_t getOldRegVal(int num);

        bool step();

        uint64_t getPC()
        {
                return getRegVal(PC);
        }

        uint64_t getSP()
        {
                return getRegVal(O6);
        }

        char * printReg(int num);

        std::ostream & outputStartState(std::ostream & os);
};

#endif
