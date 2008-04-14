#ifndef TRACECHILD_HH
#define TRACECHILD_HH

#include "regstate.hh"

class TraceChild : public RegState
{
protected:
        int pid;
        uint64_t instructions;
        bool tracing;
public:
        TraceChild() : tracing(false), instructions(0)
        {;}
        virtual bool sendState(int socket) = 0;
        virtual bool startTracing(const char * pathToFile,
                char * const argv[]);
        virtual bool stopTracing();
        virtual bool step();
        virtual uint64_t getPC() = 0;
        virtual uint64_t getSP() = 0;
        virtual std::ostream & outputStartState(std::ostream & os) = 0;
        int getPid()
        {
            return pid;
        }
        bool isTracing()
        {
                return tracing;
        }
protected:
        bool ptraceSingleStep();
        bool doWait();
};

TraceChild * genTraceChild();

#endif
