/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.

 * Modified by: JacKAsterisK
 */

#ifndef _STEAM_MSG_H
#define _STEAM_MSG_H

#define MAX_COMPRESSED_BITS       4096
#define STEAM_MSG_MAX_PLAYERS          4

#pragma pack(push, 1)

struct SteamMsg
{
   enum MsgType {
      Invalid       = 0,
      SyncRequest   = 1,
      SyncReply     = 2,
      Input         = 3,
      QualityReport = 4,
      QualityReply  = 5,
      KeepAlive     = 6,
      InputAck      = 7,
   };

   struct connect_status {
      unsigned int   disconnected:1;
      int            last_frame:31;
   };

   struct {
      ggpo::uint16         magic;
      ggpo::uint16         sequence_number;
      ggpo::uint8          type;            /* packet type */
   } hdr;
   union {
      struct {
         ggpo::uint32      random_request;  /* please reply back with this random data */
         ggpo::uint16      remote_magic;
         ggpo::uint8       remote_endpoint;
      } sync_request;
      
      struct {
         ggpo::uint32      random_reply;    /* OK, here's your random data back */
      } sync_reply;
      
      struct {
         ggpo::int8        frame_advantage; /* what's the other guy's frame advantage? */
         ggpo::uint32      ping;
      } quality_report;
      
      struct {
         ggpo::uint32      pong;
      } quality_reply;

      struct {
         connect_status    peer_connect_status[STEAM_MSG_MAX_PLAYERS];

         ggpo::uint32            start_frame;

         int               disconnect_requested:1;
         int               ack_frame:31;

         ggpo::uint16            num_bits;
         ggpo::uint8             input_size; // XXX: shouldn't be in every single packet!
         ggpo::uint8             bits[MAX_COMPRESSED_BITS]; /* must be last */
      } input;

      struct {
         int               ack_frame:31;
      } input_ack;

   } u;

public:
   int PacketSize() {
      return sizeof(hdr) + PayloadSize();
   }

   int PayloadSize() {
      int size;

      switch (hdr.type) {
      case SyncRequest:   return sizeof(u.sync_request);
      case SyncReply:     return sizeof(u.sync_reply);
      case QualityReport: return sizeof(u.quality_report);
      case QualityReply:  return sizeof(u.quality_reply);
      case InputAck:      return sizeof(u.input_ack);
      case KeepAlive:     return 0;
      case Input:
         size = (int)((char *)&u.input.bits - (char *)&u.input);
         size += (u.input.num_bits + 7) / 8;
         return size;
      }
      ASSERT(false);
      return 0;
   }

   SteamMsg(MsgType t) { hdr.type = (ggpo::uint8)t; }
};

#pragma pack(pop)

#endif   
