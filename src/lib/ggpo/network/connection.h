/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#ifndef _CONNECTION_H
#define _CONNECTION_H

#include "poll.h"
#include "connection_msg.h"
#include "ggponet.h"
#include "ring_buffer.h"

#define MAX_CONNECTION_ENDPOINTS     16

static const int MAX_CONNECTION_PACKET_SIZE = 4096;

class Connection : public IPollSink
{
public:
   struct Stats {
      int      bytes_sent;
      int      packets_sent;
      float    kbps_sent;
   };

   struct Callbacks {
      virtual ~Callbacks() { }
      virtual void OnMsg(int player_id, ConnectionMsg *msg, int len) = 0;
   };


protected:
   void Log(const char *fmt, ...);

public:
   Connection();
   void Init(Poll *p, Callbacks *callbacks, GGPOConnection* ggpo_connection);
   
   void SendTo(char *buffer, int len, int flags, int player_num);

   virtual bool OnLoopPoll(void *cookie);

public:
   ~Connection(void);

protected:
   // Network transmission information
   GGPOConnection* _ggpo_connection;
   // state management
   Callbacks      *_callbacks;
   Poll           *_poll;
};

#endif
