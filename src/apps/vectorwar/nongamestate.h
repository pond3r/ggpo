#ifndef _NON_GAMESTATE_H_
#define _NON_GAMESTATE_H_

#include "ggponet.h"

#define MAX_PLAYERS     64

/*
 * nongamestate.h --
 *
 * These are other pieces of information not related to the state
 * of the game which are useful to carry around.  They are not
 * included in the GameState class because they specifically
 * should not be rolled back.
 */

enum PlayerConnectState {
   Connecting = 0,
   Synchronizing,
   Running,
   Disconnected,
   Disconnecting,
};

struct PlayerConnectionInfo {
   GGPOPlayerType       type;
   GGPOPlayerHandle     handle;
   PlayerConnectState   state;
   int                  connect_progress;
   int                  disconnect_timeout;
   int                  disconnect_start;
};

struct NonGameState {
   struct ChecksumInfo {
      int framenumber;
      int checksum;
   };

   void SetConnectState(GGPOPlayerHandle handle, PlayerConnectState state) {
      for (int i = 0; i < num_players; i++) {
         if (players[i].handle == handle) {
            players[i].connect_progress = 0;
            players[i].state = state;
            break;
         }
      }
   }

   void SetDisconnectTimeout(GGPOPlayerHandle handle, int now, int timeout) {
      for (int i = 0; i < num_players; i++) {
         if (players[i].handle == handle) {
            players[i].disconnect_start = now;
            players[i].disconnect_timeout = timeout;
            players[i].state = Disconnecting;
            break;
         }
      }
   }

   void SetConnectState(PlayerConnectState state) {
      for (int i = 0; i < num_players; i++) {
         players[i].state = state;
      }
   }

   void UpdateConnectProgress(GGPOPlayerHandle handle, int progress) {
      for (int i = 0; i < num_players; i++) {
         if (players[i].handle == handle) {
            players[i].connect_progress = progress;
            break;
         }
      }
   }

   GGPOPlayerHandle     local_player_handle;
   PlayerConnectionInfo players[MAX_PLAYERS];
   int                  num_players;

   ChecksumInfo         now;
   ChecksumInfo         periodic;
};

#endif
