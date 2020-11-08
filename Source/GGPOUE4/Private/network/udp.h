/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#ifndef _UDP_H
#define _UDP_H

#include "../poll.h"
#include "include/connection_manager.h"

// Forward declarations
struct UdpMsg;


#define MAX_UDP_ENDPOINTS     16

static const int MAX_UDP_PACKET_SIZE = 4096;

class Udp : public IPollSink
{
public:
   struct Stats {
      int      bytes_sent;
      int      packets_sent;
      float    kbps_sent;
   };

   struct Callbacks {
      virtual ~Callbacks() { }
      virtual void OnMsg(int connection_id, UdpMsg *msg, int len) = 0;
   };

public:
   Udp();

   void Init(Poll *p, Callbacks *callbacks, ConnectionManager* connectionManager);
   
   void SendTo(char *buffer, int len, int flags, int connection_id);

   virtual bool OnLoopPoll(void *cookie);

public:
   ~Udp(void);

protected:
   ConnectionManager *_connection_manager;

   // state management
   Callbacks      *_callbacks;
   Poll           *_poll;
};

#endif
