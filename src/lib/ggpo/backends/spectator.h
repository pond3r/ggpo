/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#ifndef _SPECTATOR_H
#define _SPECTATOR_H

#include "types.h"
#include "poll.h"
#include "sync.h"
#include "backend.h"
#include "timesync.h"
#include "network/udp_proto.h"

#define SPECTATOR_FRAME_BUFFER_SIZE    64

class SpectatorBackend : public GGPOSession,  Udp::Callbacks {
public:
   SpectatorBackend(GGPOSessionCallbacks *cb, const char *gamename, uint16 localport, int num_players, int input_size, char *hostip, u_short hostport);
   virtual ~SpectatorBackend();


public:
   GGPOErrorCode DoPoll() override;
   GGPOErrorCode AddPlayer(GGPOPlayer *, GGPOPlayerHandle *) override { return GGPO_ERRORCODE_UNSUPPORTED; }
   GGPOErrorCode AddLocalInput(GGPOPlayerHandle , void *, int ) override { return GGPO_OK; }
   GGPOErrorCode SyncInput(void *, int , int *)override;
   GGPOErrorCode IncrementFrame(uint16_t)override;
   GGPOErrorCode DisconnectPlayer(GGPOPlayerHandle )override { return GGPO_ERRORCODE_UNSUPPORTED; }
   GGPOErrorCode GetNetworkStats(GGPONetworkStats *, GGPOPlayerHandle ) override { return GGPO_ERRORCODE_UNSUPPORTED; }
   GGPOErrorCode SetFrameDelay(GGPOPlayerHandle , int ) override { return GGPO_ERRORCODE_UNSUPPORTED; }
   GGPOErrorCode SetDisconnectTimeout(int ) override { return GGPO_ERRORCODE_UNSUPPORTED; }
   GGPOErrorCode SetDisconnectNotifyStart(int ) override { return GGPO_ERRORCODE_UNSUPPORTED; }
   GGPOErrorCode Chat(const char* ) override { return GGPO_ERRORCODE_UNSUPPORTED; }
   GGPOErrorCode CurrentFrame(int& ) override;
   

public:
   void OnMsg(sockaddr_in &from, UdpMsg *msg, int len) override;

protected:
   void PollUdpProtocolEvents(void);
   void CheckInitialSync(void);

   void OnUdpProtocolEvent(UdpProtocol::Event &e);

protected:
   GGPOSessionCallbacks  _callbacks;
   Poll                  _poll;
   Udp                   _udp;
   UdpProtocol           _host;
   bool                  _synchronizing;
   int                   _input_size;
   int                   _num_players;
   int                   _next_input_to_send;
   GameInput             _inputs[SPECTATOR_FRAME_BUFFER_SIZE];
};

#endif
