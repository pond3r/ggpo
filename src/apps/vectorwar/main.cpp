#include <windows.h>
#include <stdio.h>
#if defined(_DEBUG)
#   include <crtdbg.h>
#endif
#include "vectorwar.h"
#include "ggpo_perfmon.h"

int local_port, num_players, num_spectators;
GGPOPlayer *players;

LRESULT CALLBACK
MainWindowProc(HWND hwnd,
               UINT uMsg,
               WPARAM wParam,
               LPARAM lParam)
{
   switch (uMsg) {
   case WM_ERASEBKGND:
      return 1;
   case WM_KEYDOWN:
      if (wParam == 'P') {
         ggpoutil_perfmon_toggle();
      } else if (wParam == VK_ESCAPE) {
         VectorWar_Exit();
         VectorWar_Init(hwnd, local_port, num_players, players, num_spectators);
      } else if (wParam >= VK_F1 && wParam <= VK_F12) {
         VectorWar_DisconnectPlayer((int)(wParam - VK_F1));
      }
      return 0;
   case WM_PAINT:
      VectorWar_DrawCurrentFrame();
      ValidateRect(hwnd, NULL);
      return 0;
   case WM_CLOSE:
      PostQuitMessage(0);
      break;
   }
   return CallWindowProc(DefWindowProc, hwnd, uMsg, wParam, lParam);
}

HWND
CreateMainWindow(HINSTANCE hInstance)
{
   HWND hwnd;
   WNDCLASSEX wndclass = { 0 };
   RECT rc;
   int width = 640, height = 480;
   WCHAR titlebuf[128];

   wsprintf(titlebuf, L"(pid:%d) ggpo sdk sample: vector war", GetCurrentProcessId());
   wndclass.cbSize = sizeof(wndclass);
   wndclass.lpfnWndProc = MainWindowProc;
   wndclass.lpszClassName = L"vwwnd";
   RegisterClassEx(&wndclass);
   hwnd = CreateWindow(L"vwwnd",
                       titlebuf,
                       WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                       CW_USEDEFAULT, CW_USEDEFAULT,
                       width, height,
                       NULL, NULL, hInstance, NULL);

   GetClientRect(hwnd, &rc);
   SetWindowPos(hwnd, NULL, 0, 0, width + (width - (rc.right - rc.left)), height + (height - (rc.bottom - rc.top)), SWP_NOMOVE);
   return hwnd;
}

void
RunMainLoop(HWND hwnd)
{
   MSG msg = { 0 };
   int start, next, now;

   start = next = now = timeGetTime();
   while(1) {
      while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
         TranslateMessage(&msg); 
         DispatchMessage(&msg);
         if (msg.message == WM_QUIT) {
            return;
         }
      }
      now = timeGetTime();
      VectorWar_Idle(max(0, next - now - 1));
      if (now >= next) {
         VectorWar_RunFrame(hwnd);
         next = now + (1000 / 60);
      }
   }
}

void
Syntax()
{
   MessageBox(NULL, 
              L"Syntax: vectorwar.exe <local port> <num players> ('local' | <remote ip>:<remote port>)*\n",
              L"Could not start", MB_OK);
   exit(1);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
   HWND hwnd = CreateMainWindow(hInstance);
   int offset = 1, local_player = 0;
   WSADATA wd = { 0 };
   wchar_t wide_ip_buffer[128];

   WSAStartup(MAKEWORD(2, 2), &wd);
   POINT window_offsets[] = {
      { 64,  64 },   // player 1
      { 740, 64 },   // player 2
      { 64,  600 },  // player 3
      { 740, 600 },  // player 4
   };
   
#if defined(_DEBUG)
   _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

   if (__argc < 3) {
      Syntax();
      return 1;
   }
   local_port = _wtoi(__wargv[offset++]);
   num_players = _wtoi(__wargv[offset++]);
   if (num_players < 0 || __argc < offset + num_players) {
      Syntax();
   }
   if (wcscmp(__wargv[offset], L"spectate") == 0) {
      char host_ip[128];
      int host_port;
      if (swscanf(__wargv[offset+1], L"%[^:]:%d", wide_ip_buffer, &host_port) != 2) {
         Syntax();
      }
      wcstombs(host_ip, wide_ip_buffer, sizeof(host_ip));
      VectorWar_InitSpectator(hwnd, local_port, num_players, host_ip, host_port);
   } else {
      GGPOPlayer players[GGPO_MAX_SPECTATORS + GGPO_MAX_PLAYERS];

      int i;
      for (i = 0; i < num_players; i++) {
         const wchar_t *arg = __wargv[offset++];

         players[i].size = sizeof(players[i]);
         players[i].player_num = i + 1;
         if (!_wcsicmp(arg, L"local")) {
            players[i].type = GGPO_PLAYERTYPE_LOCAL;
            local_player = i;
            continue;
         }
         
         players[i].type = GGPO_PLAYERTYPE_REMOTE;
         if (swscanf(arg, L"%[^:]:%hd", wide_ip_buffer, &players[i].u.remote.port) != 2) {
            Syntax();
            return 1;
         }
         wcstombs(players[i].u.remote.ip_address, wide_ip_buffer, sizeof(players[i].u.remote.ip_address));
      }
      // these are spectators...
      num_spectators = 0;
      while (offset < __argc) {
         players[i].type = GGPO_PLAYERTYPE_SPECTATOR;
         if (swscanf(__wargv[offset++], L"%[^:]:%hd", wide_ip_buffer, &players[i].u.remote.port) != 2) {
            Syntax();
            return 1;
         }
         wcstombs(players[i].u.remote.ip_address, wide_ip_buffer, sizeof(players[i].u.remote.ip_address));
         i++;
         num_spectators++;
      }

      if (local_player < sizeof(window_offsets) / sizeof(window_offsets[0])) {
         ::SetWindowPos(hwnd, NULL, window_offsets[local_player].x, window_offsets[local_player].y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
      }

      VectorWar_Init(hwnd, local_port, num_players, players, num_spectators);
   }
   RunMainLoop(hwnd);
   VectorWar_Exit();
   delete [] players;
   WSACleanup();
   DestroyWindow(hwnd);
   return 0;
}