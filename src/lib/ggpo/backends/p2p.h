/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#ifndef _P2P_H
#define _P2P_H

#include "types.h"
#include "poll.h"
#include "sync.h"
#include "backend.h"
#include "timesync.h"
//#include "network/udp_proto.h"
#include "network/steam_proto.h"

class Peer2PeerBackend : public IQuarkBackend, IPollSink, /* Udp::Callbacks */ GGPOSteam::Callbacks {
public:
   Peer2PeerBackend(GGPOSessionCallbacks *cb, const char *gamename, ggpo::uint16 localport, int num_players, int input_size);
   virtual ~Peer2PeerBackend();


public:
   virtual GGPOErrorCode DoPoll(int timeout);
   virtual GGPOErrorCode AddPlayer(GGPOPlayer *player, GGPOPlayerHandle *handle);
   virtual GGPOErrorCode AddLocalInput(GGPOPlayerHandle player, void *values, int size);
   virtual GGPOErrorCode SyncInput(void *values, int size, int *disconnect_flags);
   virtual GGPOErrorCode IncrementFrame(void);
   virtual GGPOErrorCode DisconnectPlayer(GGPOPlayerHandle handle);
   virtual GGPOErrorCode GetNetworkStats(GGPONetworkStats *stats, GGPOPlayerHandle handle);
   virtual GGPOErrorCode SetFrameDelay(GGPOPlayerHandle player, int delay);
   virtual GGPOErrorCode SetDisconnectTimeout(int timeout);
   virtual GGPOErrorCode SetDisconnectNotifyStart(int timeout);

public:
   //virtual void OnMsg(sockaddr_in &from, UdpMsg *msg, int len);
   virtual void OnMsg(CSteamID &from, SteamMsg *msg, int len);

protected:
   GGPOErrorCode PlayerHandleToQueue(GGPOPlayerHandle player, int *queue);
   GGPOPlayerHandle QueueToPlayerHandle(int queue) { return (GGPOPlayerHandle)(queue + 1); }
   GGPOPlayerHandle QueueToSpectatorHandle(int queue) { return (GGPOPlayerHandle)(queue + 1000); } /* out of range of the player array, basically */
   void DisconnectPlayerQueue(int queue, int syncto);
   void PollSyncEvents(void);
   //void PollUdpProtocolEvents(void);
   void PollSteamProtocolEvents(void);
   void CheckInitialSync(void);
   int Poll2Players(int current_frame);
   int PollNPlayers(int current_frame);
   //void AddRemotePlayer(char *remoteip, ggpo::uint16 reportport, int queue);
   void AddRemotePlayer(CSteamID steam_id, int queue);
   //GGPOErrorCode AddSpectator(char *remoteip, ggpo::uint16 reportport);
   GGPOErrorCode AddSpectator(CSteamID steam_id);
   virtual void OnSyncEvent(Sync::Event &e) { }
//    virtual void OnUdpProtocolEvent(UdpProtocol::Event &e, GGPOPlayerHandle handle);
//    virtual void OnUdpProtocolPeerEvent(UdpProtocol::Event &e, int queue);
//    virtual void OnUdpProtocolSpectatorEvent(UdpProtocol::Event &e, int queue);
   virtual void OnSteamProtocolPeerEvent(SteamProtocol::Event &e, int queue);
   virtual void OnSteamProtocolSpectatorEvent(SteamProtocol::Event &e, int queue);
   virtual void OnSteamProtocolEvent(SteamProtocol::Event &e, GGPOPlayerHandle handle);

protected:
    GGPOSessionCallbacks  _callbacks;
    Poll                  _poll;
    Sync                  _sync;
    //    Udp                   _udp;
    //    UdpProtocol           *_endpoints;
    //    UdpProtocol           _spectators[GGPO_MAX_SPECTATORS];
    GGPOSteam             _steam;
    SteamProtocol         *_steam_endpoints;
    SteamProtocol         _steam_spectators[GGPO_MAX_SPECTATORS];
    int                   _num_spectators;
    int                   _input_size;

    bool                  _synchronizing;
    int                   _num_players;
    int                   _next_recommended_sleep;

    int                   _next_spectator_frame;
    int                   _disconnect_timeout;
    int                   _disconnect_notify_start;

   //UdpMsg::connect_status _local_connect_status[UDP_MSG_MAX_PLAYERS];
   SteamMsg::connect_status _local_connect_status[STEAM_MSG_MAX_PLAYERS];
};

#endif
