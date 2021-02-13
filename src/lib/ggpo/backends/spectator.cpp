/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#include "spectator.h"

SpectatorBackend::SpectatorBackend(GGPOSessionCallbacks *cb,
                                   const char* gamename,
                                   uint16 localport,
                                   int num_players,
                                   int input_size,
                                   char *hostip,
                                   u_short hostport) :
   _num_players(num_players),
   _input_size(input_size),
   _next_input_to_send(0)
{
   _callbacks = *cb;
   _synchronizing = true;

   for (int i = 0; i < ARRAY_SIZE(_inputs); i++) {
      _inputs[i].frame = -1;
   }

   /*
    * Initialize the UDP port
    */
   _udp.Init(localport, &_poll, this);

   /*
    * Init the host endpoint
    */
   _host.Init(&_udp, _poll, 0, hostip, hostport, NULL);
   _host.Synchronize();

   /*
    * Preload the ROM
    */
   _callbacks.begin_game(gamename, _callbacks.userdata);
}
  
SpectatorBackend::~SpectatorBackend()
{
}

GGPOErrorCode
SpectatorBackend::DoPoll(int timeout)
{
   _poll.Pump(0);

   PollUdpProtocolEvents();
   return GGPO_OK;
}

GGPOErrorCode
SpectatorBackend::SyncInput(void *values,
                            int size,
                            int *disconnect_flags)
{
   // Wait until we've started to return inputs.
   if (_synchronizing) {
      return GGPO_ERRORCODE_NOT_SYNCHRONIZED;
   }

   GameInput &input = _inputs[_next_input_to_send % SPECTATOR_FRAME_BUFFER_SIZE];
   if (input.frame < _next_input_to_send) {
      // Haven't received the input from the host yet.  Wait
      return GGPO_ERRORCODE_PREDICTION_THRESHOLD;
   }
   if (input.frame > _next_input_to_send) {
      // The host is way way way far ahead of the spectator.  How'd this
      // happen?  Anyway, the input we need is gone forever.
      return GGPO_ERRORCODE_GENERAL_FAILURE;
   }

   ASSERT(size >= _input_size * _num_players);
   memcpy(values, input.bits, _input_size * _num_players);
   if (disconnect_flags) {
      *disconnect_flags = 0; // xxx: should get them from the host!
   }
   _next_input_to_send++;

   return GGPO_OK;
}

GGPOErrorCode
SpectatorBackend::IncrementFrame(void)
{  
   Log("End of frame (%d)...\n", _next_input_to_send - 1);
   DoPoll(0);
   PollUdpProtocolEvents();

   return GGPO_OK;
}

void
SpectatorBackend::PollUdpProtocolEvents(void)
{
   UdpProtocol::Event evt;
   while (_host.GetEvent(evt)) {
      OnUdpProtocolEvent(evt);
   }
}

void
SpectatorBackend::OnUdpProtocolEvent(UdpProtocol::Event &evt)
{
   GGPOEvent info;

   switch (evt.type) {
   case UdpProtocol::Event::Connected:
      info.code = GGPO_EVENTCODE_CONNECTED_TO_PEER;
      info.u.connected.player = 0;
      _callbacks.on_event(&info, _callbacks.userdata);
      break;
   case UdpProtocol::Event::Synchronizing:
      info.code = GGPO_EVENTCODE_SYNCHRONIZING_WITH_PEER;
      info.u.synchronizing.player = 0;
      info.u.synchronizing.count = evt.u.synchronizing.count;
      info.u.synchronizing.total = evt.u.synchronizing.total;
      _callbacks.on_event(&info, _callbacks.userdata);
      break;
   case UdpProtocol::Event::Synchronzied:
      if (_synchronizing) {
         info.code = GGPO_EVENTCODE_SYNCHRONIZED_WITH_PEER;
         info.u.synchronized.player = 0;
         _callbacks.on_event(&info, _callbacks.userdata);

         info.code = GGPO_EVENTCODE_RUNNING;
         _callbacks.on_event(&info, _callbacks.userdata);
         _synchronizing = false;
      }
      break;

   case UdpProtocol::Event::NetworkInterrupted:
      info.code = GGPO_EVENTCODE_CONNECTION_INTERRUPTED;
      info.u.connection_interrupted.player = 0;
      info.u.connection_interrupted.disconnect_timeout = evt.u.network_interrupted.disconnect_timeout;
      _callbacks.on_event(&info, _callbacks.userdata);
      break;

   case UdpProtocol::Event::NetworkResumed:
      info.code = GGPO_EVENTCODE_CONNECTION_RESUMED;
      info.u.connection_resumed.player = 0;
      _callbacks.on_event(&info, _callbacks.userdata);
      break;

   case UdpProtocol::Event::Disconnected:
      info.code = GGPO_EVENTCODE_DISCONNECTED_FROM_PEER;
      info.u.disconnected.player = 0;
      _callbacks.on_event(&info, _callbacks.userdata);
      break;

   case UdpProtocol::Event::Input:
      GameInput& input = evt.u.input.input;

      _host.SetLocalFrameNumber(input.frame);
      _host.SendInputAck();
      _inputs[input.frame % SPECTATOR_FRAME_BUFFER_SIZE] = input;
      break;
   }
}
 
void
SpectatorBackend::OnMsg(sockaddr_in &from, UdpMsg *msg, int len)
{
   if (_host.HandlesMsg(from, msg)) {
      _host.OnMsg(msg, len);
   }
}

