/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#include "types.h"
#include "poll.h"

Poll::Poll(void) :
   _handle_count(0),
   _start_time(0)
{
   /*
    * Create a dummy handle to simplify things.
    */
   _handles[_handle_count++] = CreateEvent(NULL, true, false, NULL);
}

void
Poll::RegisterHandle(IPollSink *sink, HANDLE h, void *cookie)
{
   ASSERT(_handle_count < MAX_POLLABLE_HANDLES - 1);

   _handles[_handle_count] = h;
   _handle_sinks[_handle_count] = PollSinkCb(sink, cookie);
   _handle_count++;
}

void
Poll::RegisterMsgLoop(IPollSink *sink, void *cookie)
{
   _msg_sinks.push_back(PollSinkCb(sink, cookie));
}

void
Poll::RegisterLoop(IPollSink *sink, void *cookie)
{
   _loop_sinks.push_back(PollSinkCb(sink, cookie));
}
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

bool
Poll::Pump(int timeout)
{
   int i, res;
   bool finished = false;

   if (_start_time == 0) {
      _start_time = Platform::GetCurrentTimeMS();
   }
   int elapsed = Platform::GetCurrentTimeMS() - _start_time;
   int maxwait = ComputeWaitTime(elapsed);
   if (maxwait != INFINITE) {
      timeout = MIN(timeout, maxwait);
   }

   res = WaitForMultipleObjects(_handle_count, _handles, false, timeout);
   if (res >= WAIT_OBJECT_0 && res < WAIT_OBJECT_0 + _handle_count) {
      i = res - WAIT_OBJECT_0;
      finished = !_handle_sinks[i].sink->OnHandlePoll(_handle_sinks[i].cookie) || finished;
   }
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

   for (i = 0; i < _loop_sinks.size(); i++) {
      PollSinkCb &cb = _loop_sinks[i];
      finished = !cb.sink->OnLoopPoll(cb.cookie) || finished;
   }
   return finished;
}

int
Poll::ComputeWaitTime(int elapsed)
{
   int waitTime = INFINITE;
   size_t count = _periodic_sinks.size();

   if (count > 0) {
      for (int i = 0; i < count; i++) {
         PollPeriodicSinkCb &cb = _periodic_sinks[i];
         int timeout = (cb.interval + cb.last_fired) - elapsed;
         if (waitTime == INFINITE || (timeout < waitTime)) {
            waitTime = MAX(timeout, 0);
         }         
      }
   }
   return waitTime;
}
