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
#include "network/connection_proto.h"

#define SPECTATOR_FRAME_BUFFER_SIZE    64

class SpectatorBackend : public IQuarkBackend, IPollSink, Connection::Callbacks {
public:
   SpectatorBackend(GGPOSessionCallbacks *cb, const char *gamename, GGPOConnection* ggpo_connection, int num_players, int input_size, int connection_id);
   virtual ~SpectatorBackend();


public:
   virtual GGPOErrorCode DoPoll(int timeout);
   virtual GGPOErrorCode AddPlayer(GGPOPlayer *player, GGPOPlayerHandle *handle) { return GGPO_ERRORCODE_UNSUPPORTED; }
   virtual GGPOErrorCode AddLocalInput(GGPOPlayerHandle player, void *values, int size) { return GGPO_OK; }
   virtual GGPOErrorCode SyncInput(void *values, int size, int *disconnect_flags);
   virtual GGPOErrorCode IncrementFrame(void);
   virtual GGPOErrorCode DisconnectPlayer(GGPOPlayerHandle handle) { return GGPO_ERRORCODE_UNSUPPORTED; }
   virtual GGPOErrorCode GetNetworkStats(GGPONetworkStats *stats, GGPOPlayerHandle handle) { return GGPO_ERRORCODE_UNSUPPORTED; }
   virtual GGPOErrorCode SetFrameDelay(GGPOPlayerHandle player, int delay) { return GGPO_ERRORCODE_UNSUPPORTED; }
   virtual GGPOErrorCode SetDisconnectTimeout(int timeout) { return GGPO_ERRORCODE_UNSUPPORTED; }
   virtual GGPOErrorCode SetDisconnectNotifyStart(int timeout) { return GGPO_ERRORCODE_UNSUPPORTED; }

public:
   virtual void OnMsg(int player_id, ConnectionMsg *msg, int len);

protected:
   void PollConnectionProtocolEvents(void);
   void CheckInitialSync(void);

   void OnConnectionProtocolEvent(ConnectionProtocol::Event &e);

protected:
   GGPOSessionCallbacks  _callbacks;
   Poll                  _poll;
   Connection                   _connection;
   ConnectionProtocol           _host;
   bool                  _synchronizing;
   int                   _input_size;
   int                   _num_players;
   int                   _next_input_to_send;
   GameInput             _inputs[SPECTATOR_FRAME_BUFFER_SIZE];
};

#endif
