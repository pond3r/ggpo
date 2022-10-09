/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#ifndef _POLL_H
#define _POLL_H

#include "static_buffer.h"

#define MAX_POLLABLE_HANDLES     64


class IPollSink {
public:
   virtual ~IPollSink() { }
   virtual bool OnHandlePoll(void *) { return true; }
   virtual bool OnMsgPoll(void *) { return true; }
   virtual bool OnPeriodicPoll(void *, int ) { return true; }
   virtual bool OnLoopPoll(void *) { return true; }
};

class Poll {
public:
   Poll(void);
   void RegisterHandle(IPollSink *sink, HANDLE h, void *cookie = NULL);
   void RegisterMsgLoop(IPollSink *sink, void *cookie = NULL);
   void RegisterPeriodic(IPollSink *sink, int interval, void *cookie = NULL);
   void RegisterLoop(IPollSink *sink, void *cookie = NULL);

   void Run();
   bool Pump(int timeout);

protected:
   int ComputeWaitTime(int elapsed);

   struct PollSinkCb {
      IPollSink   *sink;
      void        *cookie;
      PollSinkCb() : sink(NULL), cookie(NULL) { }
      PollSinkCb(IPollSink *s, void *c) : sink(s), cookie(c) { }
   };

   struct PollPeriodicSinkCb : public PollSinkCb {
      int         interval;
      int         last_fired;
      PollPeriodicSinkCb() : PollSinkCb(NULL, NULL), interval(0), last_fired(0) { }
      PollPeriodicSinkCb(IPollSink *s, void *c, int i) :
         PollSinkCb(s, c), interval(i), last_fired(0) { }
   };

   int               _start_time;
   int               _handle_count;
   HANDLE            _handles[MAX_POLLABLE_HANDLES];
   PollSinkCb        _handle_sinks[MAX_POLLABLE_HANDLES];

   StaticBuffer<PollSinkCb, 16>          _msg_sinks;
   StaticBuffer<PollSinkCb, 16>          _loop_sinks;
   StaticBuffer<PollPeriodicSinkCb, 16>  _periodic_sinks;
};

#endif
