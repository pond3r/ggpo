#include <algorithm>
#include <locale.h>
#include <stdio.h>
#if defined(_DEBUG)
#   include <crtdbg.h>
#endif
#if defined(WIN32)
#include <Winsock2.h>
#define strcasecmp _stricmp
#endif
#include "vectorwar.h"

// #include "ggpo_perfmon.h"

int local_port, num_players, num_spectators;
GGPOPlayer *players;

void
RunMainLoop()
{
   uint32_t start, next, now;
   int input = 0;

   start = next = now = GetCurrentTimeMS();
   while (1) {
      now = GetCurrentTimeMS();
      VectorWar_Idle((std::max)((uint32_t)0, next - now - 1));
      if (now >= next) {
         input = VectorWar_RunFrame(input);
         next = now + (1000 / 60);
      }
   }
}

void
Syntax()
{
   printf("Syntax: vectorwar.exe <local port> <num players> ('local' | <remote ip>:<remote port>)*\n");
   exit(1);
}

int main(int argc, char* argv[])
{
   char* locale = setlocale(LC_ALL, "");
   if (!locale) {
     printf("could not set locale");
     exit(1);
   }


#if defined(WIN32)
   WSADATA wd = { 0 };
   WSAStartup(MAKEWORD(2, 2), &wd);
#endif

#if defined(_DEBUG)
   _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

   if (argc < 3) {
      Syntax();
      return 1;
   }
   int offset = 1, local_player = 0;

   SDL_Point window_offsets[] = {
      { 64,  64 },   // player 1
      { 740, 64 },   // player 2
      { 64,  600 },  // player 3
      { 740, 600 },  // player 4
   };

   local_port = atoi(argv[offset++]);
   num_players = atoi(argv[offset++]);
   if (num_players < 0 || argc < offset + num_players) {
      Syntax();
      return 1;
   }
   if (strcmp(argv[offset], "spectate") == 0) {
      char host_ip[128];
      int host_port;
      if (sscanf(argv[offset+1], "%[^:]:%d", host_ip, &host_port) != 2) {
         Syntax();
         return 1;
      }

      VectorWar_InitSpectator(local_port, num_players, host_ip, host_port);
   } else {
      GGPOPlayer players[GGPO_MAX_SPECTATORS + GGPO_MAX_PLAYERS];

      int i;
      for (i = 0; i < num_players; i++) {
         const char* arg = argv[offset++];

         players[i].size = sizeof(players[i]);
         players[i].player_num = i + 1;
         if (!strcasecmp(arg, "local")) {
            players[i].type = GGPO_PLAYERTYPE_LOCAL;
            local_player = i;
            continue;
         }
         
         players[i].type = GGPO_PLAYERTYPE_REMOTE;
         if (sscanf(arg, "%[^:]:%hd", players[i].u.remote.ip_address, &players[i].u.remote.port) != 2) {
            Syntax();
            return 1;
         }
      }

      // these are spectators...
      num_spectators = 0;
      while (offset < argc) {
         players[i].type = GGPO_PLAYERTYPE_SPECTATOR;
         if (sscanf(argv[offset++], "%[^:]:%hd", players[i].u.remote.ip_address, &players[i].u.remote.port) != 2) {
            Syntax();
            return 1;
         }
         i++;
         num_spectators++;
      }

      if (local_player < sizeof(window_offsets) / sizeof(window_offsets[0])) {
         // ::SetWindowPos(hwnd, NULL, window_offsets[local_player].x, window_offsets[local_player].y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
      }

      VectorWar_Init(local_port, num_players, players, num_spectators);
   }

   RunMainLoop();
   VectorWar_Exit();
#if defined(WIN32)
   WSACleanup();
#endif

   return 0;
}
