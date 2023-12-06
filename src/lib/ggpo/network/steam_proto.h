/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 * 
 * Modified by: JacKAsterisK
 */

#ifndef _STEAM_PROTO_H_
#define _STEAM_PROTO_H_

#include "poll.h"
#include "steam.h"
#include "steam_msg.h"
#include "game_input.h"
#include "timesync.h"
#include "ggponet.h"
#include "ring_buffer.h"

class SteamProtocol : public IPollSink
{
public:
   struct Stats {
      int                 ping;
      int                 remote_frame_advantage;
      int                 local_frame_advantage;
      int                 send_queue_len;
      GGPOSteam::Stats        udp;
   };

   struct Event {
      enum Type {
         Unknown = -1,
         Connected,
         Synchronizing,
         Synchronzied,
         Input,
         Disconnected,
         NetworkInterrupted,
         NetworkResumed,
      };

      Type      type;
      union {
         struct {
            GameInput   input;
         } input;
         struct {
            int         total;
            int         count;
         } synchronizing;
         struct {
            int         disconnect_timeout;
         } network_interrupted;
      } u;

      SteamProtocol::Event(Type t = Unknown) : type(t) { }
   };

public:
   virtual bool OnLoopPoll(void *cookie);

public:
   SteamProtocol();
   virtual ~SteamProtocol();

   void Init(GGPOSteam *steam, const CSteamID& remoteSteamID, Poll &p, int queue, SteamMsg::connect_status *status);

   void Synchronize();
   bool GetPeerConnectStatus(int id, int *frame);
   bool IsInitialized() { return _peer_steam_id.IsValid(); }
   bool IsSynchronized() { return _current_state == Running; }
   bool IsRunning() { return _current_state == Running; }
   void SendInput(GameInput &input);
   void SendInputAck();
   bool HandlesMsg(CSteamID &from, SteamMsg *msg);
   void OnMsg(SteamMsg *msg, int len);
   void Disconnect();
  
   void GetNetworkStats(struct GGPONetworkStats *stats);
   bool GetEvent(SteamProtocol::Event &e);
   void GGPONetworkStats(Stats *stats);
   void SetLocalFrameNumber(int num);
   int RecommendFrameDelay();

   void SetDisconnectTimeout(int timeout);
   void SetDisconnectNotifyStart(int timeout);

protected:
   enum State {
      Syncing,
      Synchronzied,
      Running,
      Disconnected
   };
   struct QueueEntry {
      int         queue_time;
      CSteamID    steam_id;
      SteamMsg      *msg;

      QueueEntry() {}
      QueueEntry(int time, CSteamID &dst, SteamMsg *m) : queue_time(time), steam_id(dst), msg(m) { }
   };

   bool CreateSocket(int retries);
   void UpdateNetworkStats(void);
   void QueueEvent(const SteamProtocol::Event &evt);
   void ClearSendQueue(void);
   void Log(const char *fmt, ...);
   void LogMsg(const char *prefix, SteamMsg *msg);
   void LogEvent(const char *prefix, const SteamProtocol::Event &evt);
   void SendSyncRequest();
   void SendMsg(SteamMsg *msg);
   void PumpSendQueue();
   void DispatchMsg(ggpo::uint8 *buffer, int len);
   void SendPendingOutput();
   bool OnInvalid(SteamMsg *msg, int len);
   bool OnSyncRequest(SteamMsg *msg, int len);
   bool OnSyncReply(SteamMsg *msg, int len);
   bool OnInput(SteamMsg *msg, int len);
   bool OnInputAck(SteamMsg *msg, int len);
   bool OnQualityReport(SteamMsg *msg, int len);
   bool OnQualityReply(SteamMsg *msg, int len);
   bool OnKeepAlive(SteamMsg *msg, int len);

protected:
   /*
    * Network transmission information
    */
   GGPOSteam      *_steam;
   CSteamID       _peer_steam_id;
   ggpo::uint16   _magic_number;
   int            _queue;
   ggpo::uint16   _remote_magic_number;
   bool           _connected;
   int            _send_latency;
   int            _oop_percent;
   struct {
      int         send_time;
      CSteamID    steam_id;
      SteamMsg*     msg;
   }              _oo_packet;
   RingBuffer<QueueEntry, 64> _send_queue;

   /*
    * Stats
    */
   int            _round_trip_time;
   int            _packets_sent;
   int            _bytes_sent;
   int            _kbps_sent;
   int            _stats_start_time;

   /*
    * The state machine
    */
   SteamMsg::connect_status *_local_connect_status;
   SteamMsg::connect_status _peer_connect_status[STEAM_MSG_MAX_PLAYERS];

   State          _current_state;
   union {
      struct {
         ggpo::uint32   roundtrips_remaining;
         ggpo::uint32   random;
      } sync;
      struct {
         ggpo::uint32   last_quality_report_time;
         ggpo::uint32   last_network_stats_interval;
         ggpo::uint32   last_input_packet_recv_time;
      } running;
   } _state;

   /*
    * Fairness.
    */
   int               _local_frame_advantage;
   int               _remote_frame_advantage;

   /*
    * Packet loss...
    */
   RingBuffer<GameInput, 64>  _pending_output;
   GameInput                  _last_received_input;
   GameInput                  _last_sent_input;
   GameInput                  _last_acked_input;
   unsigned int               _last_send_time;
   unsigned int               _last_recv_time;
   unsigned int               _shutdown_timeout;
   unsigned int               _disconnect_event_sent;
   unsigned int               _disconnect_timeout;
   unsigned int               _disconnect_notify_start;
   bool                       _disconnect_notify_sent;

   ggpo::uint16                     _next_send_seq;
   ggpo::uint16                     _next_recv_seq;

   /*
    * Rift synchronization.
    */
   TimeSync                   _timesync;

   /*
    * Event queue
    */
   RingBuffer<SteamProtocol::Event, 64>  _event_queue;
};

#endif
