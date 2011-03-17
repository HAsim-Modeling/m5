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
 * Authors: Ali Saidi
 */

#ifndef __CPU_CPUEVENT_HH__
#define __CPU_CPUEVENT_HH__

#include <vector>
#include "sim/eventq.hh"

class ThreadContext;

/**
 * This class creates a global list of events that need a pointer to a
 * thread context. When a switchover takes place the events can be
 * migrated to the new thread context, otherwise you could have a wake
 * timer interrupt go off on a switched out cpu or other unfortunate
 * events. This object MUST be dynamically allocated to avoid it being
 * deleted after a cpu switch happens.
 */
class CpuEvent : public Event
{
  protected:
    /** type of global list of cpu events. */
    typedef std::vector<CpuEvent *> CpuEventList;

    /** Static list of cpu events that is searched every time a cpu switch
     * happens. */
    static CpuEventList cpuEventList;

    /** The thread context that is switched to the new cpus. */
    ThreadContext *tc;

  public:
    CpuEvent(EventQueue *q, ThreadContext *_tc, Priority p = Default_Pri)
        : Event(q, p), tc(_tc)
    { cpuEventList.push_back(this); }

    /** delete the cpu event from the global list. */
    ~CpuEvent();

    /** Update all events switching old tc to new tc.
     * @param oldTc the old thread context we are switching from
     * @param newTc the new thread context we are switching to.
     */
    static void replaceThreadContext(ThreadContext *oldTc,
                                     ThreadContext *newTc);
    ThreadContext* getTC() { return tc; }
};

template <class T, void (T::* F)(ThreadContext *tc)>
class CpuEventWrapper : public CpuEvent
{
  private:
    T *object;

  public:
    CpuEventWrapper(T *obj, ThreadContext *_tc,
        EventQueue *q = &mainEventQueue, Priority p = Default_Pri)
        : CpuEvent(q, _tc, p), object(obj)
    { }
    void process() { (object->*F)(tc); }
};

#endif // __CPU_CPUEVENT_HH__

