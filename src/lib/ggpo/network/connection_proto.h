/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#ifndef _CONNECTION_PROTO_H_
#define _CONNECTION_PROTO_H_

#include "poll.h"
#include "connection.h"
#include "connection_msg.h"
#include "game_input.h"
#include "timesync.h"
#include "ggponet.h"
#include "ring_buffer.h"

class ConnectionProtocol : public IPollSink
{
public:
   struct Stats {
      int                 ping;
      int                 remote_frame_advantage;
      int                 local_frame_advantage;
      int                 send_queue_len;
      Connection::Stats          connection;
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

      ConnectionProtocol::Event(Type t = Unknown) : type(t) { }
   };

public:
   virtual bool OnLoopPoll(void *cookie);

public:
   ConnectionProtocol();
   virtual ~ConnectionProtocol();

   void Init(Connection *connection, Poll &p, int queue, int player_id, ConnectionMsg::connect_status *status);
   void Synchronize();
   bool GetPeerConnectStatus(int id, int *frame);
   bool IsInitialized() { return _connection != NULL; }
   bool IsSynchronized() { return _current_state == Running; }
   bool IsRunning() { return _current_state == Running; }
   void SendInput(GameInput &input);
   void SendInputAck();
   bool HandlesMsg(int player_id, ConnectionMsg *msg);
   void OnMsg(ConnectionMsg *msg, int len);
   void Disconnect();
  
   void GetNetworkStats(struct GGPONetworkStats *stats);
   bool GetEvent(ConnectionProtocol::Event &e);
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
      int         player_id;
      ConnectionMsg      *msg;

      QueueEntry() {}
      QueueEntry(int time, int playerid, ConnectionMsg *m) : queue_time(time), player_id(playerid), msg(m) { }
   };

   void UpdateNetworkStats(void);
   void QueueEvent(const ConnectionProtocol::Event &evt);
   void ClearSendQueue(void);
   void Log(const char *fmt, ...);
   void LogMsg(const char *prefix, ConnectionMsg *msg);
   void LogEvent(const char *prefix, const ConnectionProtocol::Event &evt);
   void SendSyncRequest();
   void SendMsg(ConnectionMsg *msg);
   void PumpSendQueue();
   void DispatchMsg(uint8 *buffer, int len);
   void SendPendingOutput();
   bool OnInvalid(ConnectionMsg *msg, int len);
   bool OnSyncRequest(ConnectionMsg *msg, int len);
   bool OnSyncReply(ConnectionMsg *msg, int len);
   bool OnInput(ConnectionMsg *msg, int len);
   bool OnInputAck(ConnectionMsg *msg, int len);
   bool OnQualityReport(ConnectionMsg *msg, int len);
   bool OnQualityReply(ConnectionMsg *msg, int len);
   bool OnKeepAlive(ConnectionMsg *msg, int len);

protected:
   /*
    * Network transmission information
    */
   Connection            *_connection;
   int              _player_id;
   uint16         _magic_number;
   int            _queue;
   uint16         _remote_magic_number;
   bool           _connected;
   int            _send_latency;
   int            _oop_percent;
   struct {
      int         send_time;
      int         player_id;
      ConnectionMsg*     msg;
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
   ConnectionMsg::connect_status *_local_connect_status;
   ConnectionMsg::connect_status _peer_connect_status[CONNECTION_MSG_MAX_PLAYERS];

   State          _current_state;
   union {
      struct {
         uint32   roundtrips_remaining;
         uint32   random;
      } sync;
      struct {
         uint32   last_quality_report_time;
         uint32   last_network_stats_interval;
         uint32   last_input_packet_recv_time;
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

   uint16                     _next_send_seq;
   uint16                     _next_recv_seq;

   /*
    * Rift synchronization.
    */
   TimeSync                   _timesync;

   /*
    * Event queue
    */
   RingBuffer<ConnectionProtocol::Event, 64>  _event_queue;
};

#endif
