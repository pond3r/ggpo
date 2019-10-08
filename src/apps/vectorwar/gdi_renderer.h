#ifndef _GDI_RENDERER_H_
#define _GDI_RENDERER_H_

#include "renderer.h"

/*
 * renderer.h --
 *
 * A simple C++ renderer that uses GDI to render the game state.
 *
 */

class GDIRenderer : public Renderer {
public:
   GDIRenderer(HWND hwnd);
   ~GDIRenderer();

   virtual void Draw(GameState &gs, NonGameState &ngs);
   virtual void SetStatusText(const char *text);

protected:
   void RenderChecksum(HDC hdc, int y, NonGameState::ChecksumInfo &info);
   void DrawShip(HDC hdc, int which, GameState &gamestate);
   void DrawConnectState(HDC hdc, Ship &ship, PlayerConnectionInfo &info, COLORREF color);
   void CreateGDIFont(HDC hdc);

   HFONT          _font;
   HWND           _hwnd;
   RECT           _rc;
   HGLRC          _hrc;
   char           _status[1024];
   COLORREF       _shipColors[4];
   HPEN           _shipPens[4];
   HBRUSH         _bulletBrush;
   HBRUSH         _redBrush;
};

#endif
