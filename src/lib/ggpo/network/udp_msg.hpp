/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#ifndef _UDP_MSG_H
#define _UDP_MSG_H

#include <cstdint>
#include <types.hpp>

#define MAX_COMPRESSED_BITS 4096
#define UDP_MSG_MAX_PLAYERS 4

#pragma pack(push, 1)

struct UdpMsg
{
   enum MsgType
   {
      Invalid = 0,
      SyncRequest = 1,
      SyncReply = 2,
      Input = 3,
      QualityReport = 4,
      QualityReply = 5,
      KeepAlive = 6,
      InputAck = 7,
   };

   struct connect_status
   {
      int disconnected : 1;
      int last_frame : 31;
   };

   struct
   {
      uint16_t magic;
      uint16_t sequence_number;
      uint8_t type; /* packet type */
   } hdr;
   union {
      struct
      {
         uint32_t random_request; /* please reply back with this random data */
         uint16_t remote_magic;
         uint8_t remote_endpoint;
      } sync_request;

      struct
      {
         uint32_t random_reply; /* OK, here's your random data back */
      } sync_reply;

      struct
      {
         int8_t frame_advantage; /* what's the other guy's frame advantage? */
         uint32_t ping;
      } quality_report;

      struct
      {
         uint32_t pong;
      } quality_reply;

      struct
      {
         connect_status peer_connect_status[UDP_MSG_MAX_PLAYERS];

         uint32_t start_frame;

         int disconnect_requested : 1;
         int ack_frame : 31;

         uint16_t num_bits;
         uint8_t input_size;                // XXX: shouldn't be in every single packet!
         uint8_t bits[MAX_COMPRESSED_BITS]; /* must be last */
      } input;

      struct
      {
         int ack_frame : 31;
      } input_ack;

   } u;

public:
   int PacketSize()
   {
      return sizeof(hdr) + PayloadSize();
   }

   int PayloadSize()
   {
      int size;

      switch (hdr.type)
      {
      case SyncRequest:
         return sizeof(u.sync_reply);
      case SyncReply:
         return sizeof(u.sync_reply);
      case QualityReport:
         return sizeof(u.quality_report);
      case QualityReply:
         return sizeof(u.quality_reply);
      case InputAck:
         return sizeof(u.input_ack);
      case KeepAlive:
         return 0;
      case Input:
         size = (int)((char *)&u.input.bits - (char *)&u.input);
         size += (u.input.num_bits + 7) / 8;
         return size;
      }
      ASSERT(false);
      return 0;
   }

   UdpMsg(MsgType t) { hdr.type = t; }
};

#pragma pack(pop)

#endif