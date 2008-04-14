#ifndef REGSTATE_H
#define REGSTATE_H

#include <string>
#include <stdint.h>

class RegState
{
protected:
        virtual bool update(int pid) = 0;
public:
        virtual int getNumRegs() = 0;
        virtual bool diffSinceUpdate(int num) = 0;
        virtual std::string getRegName(int num) = 0;
        virtual int getRegNum(std::string name)
        {
                int numregs = getNumRegs();
                for(unsigned int x = 0; x < numregs; x++)
                        if(getRegName(x) == name)
                                return x;
                return -1;
        }
        virtual int64_t getRegVal(int num) = 0;
        virtual int64_t getOldRegVal(int num) = 0;
        virtual char * printReg(int num) = 0;
};

#endif
