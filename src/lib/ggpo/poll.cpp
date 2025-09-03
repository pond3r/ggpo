/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#include "types.h"
#include "poll.h"


void Poll::RegisterLoop(IPollSink *sink)
{
   _loop_sinks.push_back(sink);
}

bool Poll::Pump()
{
   int i;
   bool finished = false;

   for (i = 0; i < _loop_sinks.size(); i++) {
      auto *sink = _loop_sinks[i];
      finished = !sink->OnLoopPoll() || finished;
   }
   return finished;
}