/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.

 * Modified by: JacKAsterisK
 */

#ifndef _STEAM_H
#define _STEAM_H

#include "poll.h"
#include "steam_msg.h"
#include "ggponet.h"
#include "ring_buffer.h"

#define MAX_STEAM_ENDPOINTS     16

static const int MAX_STEAM_PACKET_SIZE = 4096;

class GGPOSteam : public IPollSink
{
public:
   struct Stats {
      int      bytes_sent;
      int      packets_sent;
      float    kbps_sent;
   };

   struct Callbacks {
      virtual ~Callbacks() { }
      virtual void OnMsg(CSteamID &from, SteamMsg *msg, int len) = 0;
   };

protected:
	CSteamID _local_steam_id;

	// state management
	Callbacks* _callbacks;
	Poll* _poll;

public:
   GGPOSteam();
   ~GGPOSteam(void);

   CSteamID GetLocalSteamID() { return _local_steam_id; }

   void Init(Poll *p, Callbacks *callbacks);
   
   void SendTo(char *buffer, int len, EP2PSend flags, CSteamID &dst);

   virtual bool OnLoopPoll(void *cookie);

protected:
	void Log(const char* fmt, ...);
};

#endif
