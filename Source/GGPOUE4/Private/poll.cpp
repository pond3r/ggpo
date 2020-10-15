/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#include "poll.h"
#include "types.h"

Poll::Poll(void) 
#if POLL_FEATURES
	:
   _handle_count(0),
   _start_time(0)
#endif
{

#if POLL_FEATURES
#ifdef _WIN32
	/*
    * Create a dummy handle to simplify things.
    */
   _handles[_handle_count++] = CreateEvent(NULL, true, false, NULL);
#else
	// This case is unused and can safely be omitted if Register Handle
	// is never called or used.
#endif
#endif
}

#if POLL_FEATURES
void
Poll::RegisterHandle(IPollSink *sink, ggpo_handle_t h, void *cookie)
{
	check(_handle_count < MAX_POLLABLE_HANDLES - 1);

   _handles[_handle_count] = h;
   _handle_sinks[_handle_count] = PollSinkCb(sink, cookie);
   _handle_count++;
}

void
Poll::RegisterMsgLoop(IPollSink *sink, void *cookie)
{
   _msg_sinks.push_back(PollSinkCb(sink, cookie));
}
#endif

void
Poll::RegisterLoop(IPollSink *sink, void *cookie)
{
   _loop_sinks.push_back(PollSinkCb(sink, cookie));
}

#if POLL_FEATURES
void
Poll::RegisterPeriodic(IPollSink *sink, int interval, void *cookie)
{
   _periodic_sinks.push_back(PollPeriodicSinkCb(sink, cookie, interval));
}

void
Poll::Run()
{
   while (Pump(100)) {
      continue;
   }
}
#endif

bool
Poll::Pump(int timeout)
{
   bool finished = false;
   int i;

#if POLL_FEATURES
#ifdef _WIN32
   int res;
#endif

   if (_start_time == 0) {
      _start_time = PlatformGGPO::GetCurrentTimeMS();
   }
   int elapsed = PlatformGGPO::GetCurrentTimeMS() - _start_time;
   int maxwait = ComputeWaitTime(elapsed);
   if (maxwait != GGPO_INF) {
      timeout = MIN(timeout, maxwait);
   }

#ifdef _WIN32
   res = WaitForMultipleObjects(_handle_count, _handles, false, timeout);
   if (res >= WAIT_OBJECT_0 && res < WAIT_OBJECT_0 + _handle_count) {
      i = res - WAIT_OBJECT_0;
      finished = !_handle_sinks[i].sink->OnHandlePoll(_handle_sinks[i].cookie) || finished;
   }
#endif

   for (i = 0; i < _msg_sinks.size(); i++) {
      PollSinkCb &cb = _msg_sinks[i];
      finished = !cb.sink->OnMsgPoll(cb.cookie) || finished;
   }

   for (i = 0; i < _periodic_sinks.size(); i++) {
      PollPeriodicSinkCb &cb = _periodic_sinks[i];
      if (cb.interval + cb.last_fired <= elapsed) {
         cb.last_fired = (elapsed / cb.interval) * cb.interval;
         finished = !cb.sink->OnPeriodicPoll(cb.cookie, cb.last_fired) || finished;
      }
   }
#endif

   for (i = 0; i < _loop_sinks.size(); i++) {
      PollSinkCb &cb = _loop_sinks[i];
      finished = !cb.sink->OnLoopPoll(cb.cookie) || finished;
   }
   return finished;
}

#if POLL_FEATURES
int
Poll::ComputeWaitTime(int elapsed)
{
   int waitTime = GGPO_INF;
   size_t count = _periodic_sinks.size();

   if (count > 0) {
      for (int i = 0; i < count; i++) {
         PollPeriodicSinkCb &cb = _periodic_sinks[i];
         int timeout = (cb.interval + cb.last_fired) - elapsed;
         if (waitTime == GGPO_INF || (timeout < waitTime)) {
            waitTime = MAX(timeout, 0);
         }         
      }
   }
   return waitTime;
}
#endif