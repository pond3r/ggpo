/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#ifndef _POLL_H
#define _POLL_H

#include "static_buffer.h"



class IPollSink {
public:
   virtual ~IPollSink() { }
   virtual bool OnLoopPoll() = 0;
};

class Poll {
public:
   void RegisterLoop(IPollSink *sink);
   bool Pump();

protected:
   StaticBuffer<IPollSink*, 16>          _loop_sinks;
};

#endif
