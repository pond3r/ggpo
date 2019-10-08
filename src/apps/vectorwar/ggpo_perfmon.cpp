#include <windows.h>
#include <stdio.h>
#include "resource.h"
#include "ggponet.h"
#include "ggpo_perfmon.h"

#define MAX_GRAPH_SIZE      4096
#define MAX_FAIRNESS          20
#define MAX_PLAYERS            4

static HWND _hwnd = NULL;
static HWND _dialog = NULL;
static HPEN _green_pen, _red_pen, _blue_pen, _yellow_pen, _grey_pen, _pink_pen, _fairness_pens[MAX_PLAYERS];
static BOOL _shown = FALSE;
static int _last_text_update_time = 0;

int _num_players;
int _first_graph_index = 0;
int _graph_size = 0;
int _ping_graph[MAX_PLAYERS][MAX_GRAPH_SIZE];
int _local_fairness_graph[MAX_PLAYERS][MAX_GRAPH_SIZE];
int _remote_fairness_graph[MAX_PLAYERS][MAX_GRAPH_SIZE];
int _fairness_graph[MAX_GRAPH_SIZE];
int _predict_queue_graph[MAX_GRAPH_SIZE];
int _remote_queue_graph[MAX_GRAPH_SIZE];
int _send_queue_graph[MAX_GRAPH_SIZE];

static void
draw_graph(LPDRAWITEMSTRUCT di, HPEN pen, int graph[],
           int count, int min, int max)
{
   POINT pt[MAX_GRAPH_SIZE];
   int i, height = di->rcItem.bottom - di->rcItem.top;
   int width = di->rcItem.right - di->rcItem.left;
   int range = max - min, offset = 0;

   if (count > width) {
      offset = count - width;
      count = width;
   }
   for (i = 0; i < count; i++) {
      int value = graph[(_first_graph_index + offset + i) % MAX_GRAPH_SIZE] - min;
      int y = height - (value * height / range);
      pt[i].x = (width - count) + i;
      pt[i].y = y;
   }  
   SelectObject(di->hDC, pen);
   Polyline(di->hDC, pt, count);
}

static void
draw_grid(LPDRAWITEMSTRUCT di)
{  
   FillRect(di->hDC, &di->rcItem, (HBRUSH)GetStockObject(BLACK_BRUSH));
}

static void
draw_network_graph_control(LPDRAWITEMSTRUCT di)
{
   draw_grid(di);
   for (int i = 0; i < _num_players; i++) {
      draw_graph(di, _green_pen, _ping_graph[i],          _graph_size, 0, 500);
   }
   draw_graph(di, _pink_pen,  _predict_queue_graph, _graph_size, 0, 14);
   draw_graph(di, _red_pen,   _remote_queue_graph,  _graph_size, 0, 14);
   draw_graph(di, _blue_pen,  _send_queue_graph,    _graph_size, 0, 14);

   _fairness_pens[0] = _blue_pen;
   _fairness_pens[1] = _grey_pen;
   _fairness_pens[2] = _red_pen;
   _fairness_pens[3] = _pink_pen;
}

static void
draw_fairness_graph_control(LPDRAWITEMSTRUCT di)
{
   int midpoint = (di->rcItem.bottom - di->rcItem.top) / 2;

   draw_grid(di);
   SelectObject(di->hDC, _grey_pen);

   MoveToEx(di->hDC, di->rcItem.left, midpoint, NULL);
   LineTo(di->hDC, di->rcItem.right, midpoint);

   for (int i = 0; i < _num_players; i++) {
      draw_graph(di, _fairness_pens[i],    _remote_fairness_graph[i], _graph_size, -MAX_FAIRNESS, MAX_FAIRNESS);
      //draw_graph(di, _blue_pen,   _local_fairness_graph,  _graph_size, -MAX_FAIRNESS, MAX_FAIRNESS);
   }
   draw_graph(di, _yellow_pen, _fairness_graph,        _graph_size, -MAX_FAIRNESS, MAX_FAIRNESS);
}

static INT_PTR CALLBACK
ggpo_perfmon_dlgproc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

   switch (uMsg) {
   case WM_COMMAND:
      {
         ggpoutil_perfmon_toggle();
         return TRUE;
      }
      break;

   case WM_DRAWITEM:
      {
         LPDRAWITEMSTRUCT lpDrawItem = (LPDRAWITEMSTRUCT) lParam;
         if (lpDrawItem->CtlID == IDC_FAIRNESS_GRAPH) {
            draw_fairness_graph_control(lpDrawItem);
         } else {
            draw_network_graph_control(lpDrawItem);
         }
         return TRUE;
      }
   case WM_INITDIALOG:
      {
         char pid[64];
         sprintf(pid, "%d", GetCurrentProcessId());
         SetWindowTextA(GetDlgItem(hwndDlg, IDC_PID), pid);   
         return TRUE;
      }
   }
   return FALSE;
}

void
ggpoutil_perfmon_init(HWND hwnd)
{
   _hwnd = hwnd;
   _green_pen = CreatePen(PS_SOLID, 1, RGB(0, 255, 0));
   _red_pen = CreatePen(PS_SOLID, 1, RGB(255, 64, 64));
   _blue_pen = CreatePen(PS_SOLID, 1, RGB(64, 64, 255));
   _yellow_pen = CreatePen(PS_SOLID, 1, RGB(255, 235, 0));
   _grey_pen = CreatePen(PS_SOLID, 1, RGB(96, 96, 96));
   _pink_pen = CreatePen(PS_SOLID, 1, RGB(255, 0, 255));
}

void
ggpoutil_perfmon_exit()
{
   DeleteObject(_green_pen);
   DeleteObject(_red_pen);
   DeleteObject(_blue_pen);
   DeleteObject(_yellow_pen);
   DeleteObject(_grey_pen);
   DeleteObject(_pink_pen);
}

void
ggpoutil_perfmon_toggle()
{
   if (!_dialog) {
      _dialog = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_PERFMON),
                             _hwnd, ggpo_perfmon_dlgproc);
   }
   _shown = !_shown;
   ShowWindow(_dialog, _shown ? SW_SHOW : SW_HIDE);
}

void
ggpoutil_perfmon_update(GGPOSession *ggpo, GGPOPlayerHandle players[], int num_players)
{
   GGPONetworkStats stats;
   int i;

   _num_players = num_players;

   if (_graph_size < MAX_GRAPH_SIZE) {
      i = _graph_size++;
   } else {
      i = _first_graph_index;
      _first_graph_index = (_first_graph_index + 1) % MAX_GRAPH_SIZE;
   }

   /*
    * Random graphs
    */
   //_predict_queue_graph[i] = stats.network.predict_queue_len;
   //_remote_queue_graph[i] = stats.network.recv_queue_len;
   //_send_queue_graph[i] = stats.network.send_queue_len;


   for (int j = 0; j < num_players; j++) {
      ggpo_get_network_stats(ggpo, players[j], &stats);

      /*
       * Ping
       */
      _ping_graph[j][i] = stats.network.ping;

      /*
       * Frame Advantage
       */
      _local_fairness_graph[j][i] = stats.timesync.local_frames_behind;
      _remote_fairness_graph[j][i] = stats.timesync.remote_frames_behind;
      if (stats.timesync.local_frames_behind < 0 && stats.timesync.remote_frames_behind < 0) {
         /*
          * Both think it's unfair (which, ironically, is fair).  Scale both and subtrace.
          */
         _fairness_graph[i] = abs(abs(stats.timesync.local_frames_behind) - abs(stats.timesync.remote_frames_behind));
      } else if (stats.timesync.local_frames_behind > 0 && stats.timesync.remote_frames_behind > 0) {
         /*
          * Impossible!  Unless the network has negative transmit time.  Odd....
          */
         _fairness_graph[i] = 0;
      } else {
         /*
          * They disagree.  Add.
          */
         _fairness_graph[i] = abs(stats.timesync.local_frames_behind) + abs(stats.timesync.remote_frames_behind);
      }
   }

   int now = timeGetTime();
   if (_dialog) {
      InvalidateRect(GetDlgItem(_dialog, IDC_FAIRNESS_GRAPH), NULL, FALSE);
      InvalidateRect(GetDlgItem(_dialog, IDC_NETWORK_GRAPH), NULL, FALSE);

      if (now > _last_text_update_time + 500) {
         char fLocal[128], fRemote[128], fBandwidth[128];
         char msLag[128], frameLag[128];
 
         sprintf(msLag, "%d ms", stats.network.ping);
         sprintf(frameLag, "%.1f frames", stats.network.ping ? stats.network.ping * 60.0 / 1000 : 0);
         sprintf(fBandwidth, "%.2f kilobytes/sec", stats.network.kbps_sent / 8.0);
         sprintf(fLocal, "%d frames", stats.timesync.local_frames_behind);
         sprintf(fRemote, "%d frames", stats.timesync.remote_frames_behind);
         SetWindowTextA(GetDlgItem(_dialog, IDC_NETWORK_LAG), msLag);
         SetWindowTextA(GetDlgItem(_dialog, IDC_FRAME_LAG), frameLag);
         SetWindowTextA(GetDlgItem(_dialog, IDC_BANDWIDTH), fBandwidth);
         SetWindowTextA(GetDlgItem(_dialog, IDC_LOCAL_AHEAD), fLocal);
         SetWindowTextA(GetDlgItem(_dialog, IDC_REMOTE_AHEAD), fRemote);
         _last_text_update_time = now;
      }
   }
}

