/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#include "types.h"
#include "backends/p2p.h"
#include "backends/synctest.h"
#include "backends/spectator.h"
#include "include/ggponet.h"

BOOL WINAPI
DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
   srand(Platform::GetCurrentTimeMS() + Platform::GetProcessID());
   return true;
}

void
GGPONet::ggpo_log(GGPOSession *ggpo, const char *fmt, ...)
{
   va_list args;
   va_start(args, fmt);
   ggpo_logv(ggpo, fmt, args);
   va_end(args);
}

void
GGPONet::ggpo_logv(GGPOSession *ggpo, const char *fmt, va_list args)
{
   if (ggpo) {
      ggpo->Logv(fmt, args);
   }
}

GGPOErrorCode
GGPONet::ggpo_start_session(GGPOSession **session,
                   GGPOSessionCallbacks *cb,
                   const char *game,
                   int num_players,
                   int input_size,
                   unsigned short localport)
{
   *session= (GGPOSession *)new Peer2PeerBackend(cb,
                                                 game,
                                                 localport,
                                                 num_players,
                                                 input_size);
   return GGPO_OK;
}

GGPOErrorCode
GGPONet::ggpo_add_player(GGPOSession *ggpo,
                GGPOPlayer *player,
                GGPOPlayerHandle *handle)
{
   if (!ggpo) {
      return GGPO_ERRORCODE_INVALID_SESSION;
   }
   return ggpo->AddPlayer(player, handle);
}



GGPOErrorCode
GGPONet::ggpo_start_synctest(GGPOSession **ggpo,
                    GGPOSessionCallbacks *cb,
                    char *game,
                    int num_players,
                    int input_size,
                    int frames)
{
   *ggpo = (GGPOSession *)new SyncTestBackend(cb, game, frames, num_players);
   return GGPO_OK;
}

GGPOErrorCode
GGPONet::ggpo_set_frame_delay(GGPOSession *ggpo,
                     GGPOPlayerHandle player,
                     int frame_delay)
{
   if (!ggpo) {
      return GGPO_ERRORCODE_INVALID_SESSION;
   }
   return ggpo->SetFrameDelay(player, frame_delay);
}

GGPOErrorCode
GGPONet::ggpo_idle(GGPOSession *ggpo, int timeout)
{
   if (!ggpo) {
      return GGPO_ERRORCODE_INVALID_SESSION;
   }
   return ggpo->DoPoll(timeout);
}

GGPOErrorCode
GGPONet::ggpo_add_local_input(GGPOSession *ggpo,
                     GGPOPlayerHandle player,
                     void *values,
                     int size)
{
   if (!ggpo) {
      return GGPO_ERRORCODE_INVALID_SESSION;
   }
   return ggpo->AddLocalInput(player, values, size);
}

GGPOErrorCode
GGPONet::ggpo_synchronize_input(GGPOSession *ggpo,
                       void *values,
                       int size,
                       int *disconnect_flags)
{
   if (!ggpo) {
      return GGPO_ERRORCODE_INVALID_SESSION;
   }
   return ggpo->SyncInput(values, size, disconnect_flags);
}

GGPOErrorCode GGPONet::ggpo_disconnect_player(GGPOSession *ggpo,
                                     GGPOPlayerHandle player)
{
   if (!ggpo) {
      return GGPO_ERRORCODE_INVALID_SESSION;
   }
   return ggpo->DisconnectPlayer(player);
}

GGPOErrorCode
GGPONet::ggpo_advance_frame(GGPOSession *ggpo)
{
   if (!ggpo) {
      return GGPO_ERRORCODE_INVALID_SESSION;
   }
   return ggpo->IncrementFrame();
}

GGPOErrorCode
ggpo_client_chat(GGPOSession *ggpo, char *text)
{
   if (!ggpo) {
      return GGPO_ERRORCODE_INVALID_SESSION;
   }
   return ggpo->Chat(text);
}

GGPOErrorCode
GGPONet::ggpo_get_network_stats(GGPOSession *ggpo,
                       GGPOPlayerHandle player,
                       FGGPONetworkStats *stats)
{
   if (!ggpo) {
      return GGPO_ERRORCODE_INVALID_SESSION;
   }
   return ggpo->GetNetworkStats(stats, player);
}


GGPOErrorCode
GGPONet::ggpo_close_session(GGPOSession *ggpo)
{
   if (!ggpo) {
      return GGPO_ERRORCODE_INVALID_SESSION;
   }
   delete ggpo;
   return GGPO_OK;
}

GGPOErrorCode
GGPONet::ggpo_set_disconnect_timeout(GGPOSession *ggpo, int timeout)
{
   if (!ggpo) {
      return GGPO_ERRORCODE_INVALID_SESSION;
   }
   return ggpo->SetDisconnectTimeout(timeout);
}

GGPOErrorCode
GGPONet::ggpo_set_disconnect_notify_start(GGPOSession *ggpo, int timeout)
{
   if (!ggpo) {
      return GGPO_ERRORCODE_INVALID_SESSION;
   }
   return ggpo->SetDisconnectNotifyStart(timeout);
}

GGPOErrorCode
GGPONet::ggpo_try_synchronize_local(GGPOSession* ggpo)
{
    if (!ggpo) {
        return GGPO_ERRORCODE_INVALID_SESSION;
    }
    return ggpo->TrySynchronizeLocal();
}

GGPOErrorCode GGPONet::ggpo_start_spectating(GGPOSession **session,
                                    GGPOSessionCallbacks *cb,
                                    const char *game,
                                    int num_players,
                                    int input_size,
                                    unsigned short local_port,
                                    char *host_ip,
                                    unsigned short host_port)
{
   *session= (GGPOSession *)new SpectatorBackend(cb,
                                                 game,
                                                 local_port,
                                                 num_players,
                                                 input_size,
                                                 host_ip,
                                                 host_port);
   return GGPO_OK;
}

