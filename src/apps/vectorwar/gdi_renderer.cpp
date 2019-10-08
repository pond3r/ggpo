#include <windows.h>
#include <stdio.h>
#include <math.h>
#include "vectorwar.h"
#include "gdi_renderer.h"

#define  PROGRESS_BAR_WIDTH        100
#define  PROGRESS_BAR_TOP_OFFSET    22
#define  PROGRESS_BAR_HEIGHT         8
#define  PROGRESS_TEXT_OFFSET       (PROGRESS_BAR_TOP_OFFSET + PROGRESS_BAR_HEIGHT + 4)

GDIRenderer::GDIRenderer(HWND hwnd) :
   _hwnd(hwnd)
{
   HDC hdc = GetDC(_hwnd);
   *_status = '\0';
   GetClientRect(hwnd, &_rc);
   CreateGDIFont(hdc);
   ReleaseDC(_hwnd, hdc);

   _shipColors[0] = RGB(255, 0, 0);
   _shipColors[1] = RGB(0, 255, 0);
   _shipColors[2] = RGB(0, 0, 255);
   _shipColors[3] = RGB(128, 128, 128);
   
   for (int i = 0; i < 4; i++) {
      _shipPens[i] = CreatePen(PS_SOLID, 1, _shipColors[i]);
   }
   _redBrush = CreateSolidBrush(RGB(255, 0, 0));
   _bulletBrush = CreateSolidBrush(RGB(255, 192, 0));
}


GDIRenderer::~GDIRenderer()
{
   DeleteObject(_font);
}


void
GDIRenderer::Draw(GameState &gs, NonGameState &ngs)
{
   HDC hdc = GetDC(_hwnd);

   FillRect(hdc, &_rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
   FrameRect(hdc, &gs._bounds, (HBRUSH)GetStockObject(WHITE_BRUSH));

   SetBkMode(hdc, TRANSPARENT);
   SelectObject(hdc, _font);

   for (int i = 0; i < gs._num_ships; i++) {
      SetTextColor(hdc, _shipColors[i]);
      SelectObject(hdc, _shipPens[i]);
      DrawShip(hdc, i, gs);
      DrawConnectState(hdc, gs._ships[i], ngs.players[i], _shipColors[i]);
   }

   SetTextAlign(hdc, TA_BOTTOM | TA_CENTER);
   TextOutA(hdc, (_rc.left + _rc.right) / 2, _rc.bottom - 32, _status, strlen(_status));

   SetTextColor(hdc, RGB(192, 192, 192));
   RenderChecksum(hdc, 40, ngs.periodic);
   SetTextColor(hdc, RGB(128, 128, 128));
   RenderChecksum(hdc, 56, ngs.now);

   //SwapBuffers(hdc);
   ReleaseDC(_hwnd, hdc);
}

void
GDIRenderer::RenderChecksum(HDC hdc, int y, NonGameState::ChecksumInfo &info)
{
   char checksum[128];
   sprintf(checksum, "Frame: %04d  Checksum: %08x", info.framenumber, info.checksum);
   TextOutA(hdc, (_rc.left + _rc.right) / 2, _rc.top + y, checksum, strlen(checksum));
}


void
GDIRenderer::SetStatusText(const char *text)
{
   strcpy(_status, text);
}

void
GDIRenderer::DrawShip(HDC hdc, int which, GameState &gs)
{
   Ship *ship = gs._ships + which;
   RECT bullet = { 0 };
   POINT shape[] = {
      { SHIP_RADIUS,           0 },
      { -SHIP_RADIUS,          SHIP_WIDTH },
      { SHIP_TUCK-SHIP_RADIUS, 0 },
      { -SHIP_RADIUS,          -SHIP_WIDTH },
      { SHIP_RADIUS,           0 },
   };
   int alignments[] = {
      TA_TOP | TA_LEFT,
      TA_TOP | TA_RIGHT,
      TA_BOTTOM | TA_LEFT,
      TA_BOTTOM | TA_RIGHT,
   };
   POINT text_offsets[] = {
      { gs._bounds.left  + 2, gs._bounds.top + 2 },
      { gs._bounds.right - 2, gs._bounds.top + 2 },
      { gs._bounds.left  + 2, gs._bounds.bottom - 2 },
      { gs._bounds.right - 2, gs._bounds.bottom - 2 },
   };
   char buf[32];
   int i;

   for (i = 0; i < ARRAY_SIZE(shape); i++) {
      int newx, newy;
      double cost, sint, theta;

      theta = (double)ship->heading * PI / 180;
      cost = ::cos(theta);
      sint = ::sin(theta);

      newx = shape[i].x * cost - shape[i].y * sint;
      newy = shape[i].x * sint + shape[i].y * cost;

      shape[i].x = newx + ship->position.x;
      shape[i].y = newy + ship->position.y;
   }
   Polyline(hdc, shape, ARRAY_SIZE(shape));

   for (int i = 0; i < MAX_BULLETS; i++) {
      if (ship->bullets[i].active) {
         bullet.left = ship->bullets[i].position.x - 1;
         bullet.right = ship->bullets[i].position.x + 1;
         bullet.top = ship->bullets[i].position.y - 1;
         bullet.bottom = ship->bullets[i].position.y + 1;
         FillRect(hdc, &bullet, _bulletBrush);
      }
   }
   SetTextAlign(hdc, alignments[which]);
   sprintf(buf, "Hits: %d", ship->score);
   TextOutA(hdc, text_offsets[which].x, text_offsets[which].y, buf, strlen(buf));
}

void
GDIRenderer::DrawConnectState(HDC hdc, Ship &ship, PlayerConnectionInfo &info, COLORREF color)
{
   char status[64];
   static const char *statusStrings[] = {
      "Connecting...",
      "Synchronizing...",
      "",
      "Disconnected.",
   };
   int progress = -1;

   *status = '\0';
   switch (info.state) {
      case Connecting:
         sprintf(status, (info.type == GGPO_PLAYERTYPE_LOCAL) ? "Local Player" : "Connecting...");
         break;

      case Synchronizing:
         progress = info.connect_progress;
         sprintf(status, (info.type == GGPO_PLAYERTYPE_LOCAL) ? "Local Player" : "Synchronizing...");
         break;

      case Disconnected:
         sprintf(status, "Disconnected");
         break;

      case Disconnecting:
         sprintf(status, "Waiting for player...");
         progress = (timeGetTime() - info.disconnect_start) * 100 / info.disconnect_timeout;
         break;
   }

   if (*status) {
      SetTextAlign(hdc, TA_TOP | TA_CENTER);
      TextOutA(hdc, ship.position.x, ship.position.y + PROGRESS_TEXT_OFFSET, status, strlen(status));
   }
   if (progress >= 0) {
      HBRUSH bar = (HBRUSH)(info.state == Synchronizing ? GetStockObject(WHITE_BRUSH) : _redBrush);
      RECT rc = { ship.position.x - (PROGRESS_BAR_WIDTH / 2),
                  ship.position.y + PROGRESS_BAR_TOP_OFFSET,
                  ship.position.x + (PROGRESS_BAR_WIDTH / 2),
                  ship.position.y + PROGRESS_BAR_TOP_OFFSET + PROGRESS_BAR_HEIGHT };

      FrameRect(hdc, &rc, (HBRUSH)GetStockObject(GRAY_BRUSH));
      rc.right = rc.left + min(100, progress) * PROGRESS_BAR_WIDTH / 100;
      InflateRect(&rc, -1, -1);
      FillRect(hdc, &rc, bar);
   }
}


void
GDIRenderer::CreateGDIFont(HDC hdc)
{
   _font = CreateFont(-12,
                      0,                         // Width Of Font
                      0,                         // AnGDIe Of Escapement
                      0,                         // Orientation AnGDIe
                      0,                         // Font Weight
                      FALSE,                     // Italic
                      FALSE,                     // Underline
                      FALSE,                     // Strikeout
                      ANSI_CHARSET,              // Character Set Identifier
                      OUT_TT_PRECIS,             // Output Precision
                      CLIP_DEFAULT_PRECIS,       // Clipping Precision
                      ANTIALIASED_QUALITY,       // Output Quality
                      FF_DONTCARE|DEFAULT_PITCH,	// Family And Pitch
                      L"Tahoma");                // Font Name

}
