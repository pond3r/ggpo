#include <stdio.h>
#include <math.h>
#include <algorithm>

#include "font.h"
#include "vectorwar.h"
#include "sdl_renderer.h"
#include "gamestate.h"

#define  PROGRESS_BAR_WIDTH        100
#define  PROGRESS_BAR_TOP_OFFSET    22
#define  PROGRESS_BAR_HEIGHT         8
#define  PROGRESS_TEXT_OFFSET       (PROGRESS_BAR_TOP_OFFSET + PROGRESS_BAR_HEIGHT + 4)

SDL_Window*
CreateMainWindow()
{
   SDL_Window *window;

   char titlebuf[128];
   snprintf(titlebuf, strlen(titlebuf),
      "(pid: %d) ggpo sdk sample: vector war", GetProcessID());

   window = SDL_CreateWindow(
       titlebuf,
       SDL_WINDOWPOS_UNDEFINED,           // initial x position
       SDL_WINDOWPOS_UNDEFINED,           // initial y position
       640,                               // width, in pixels
       480,                               // height, in pixels
       SDL_WINDOW_SHOWN
   );

   return window;
}

void print_SDL_error(const char* loc)
{
   printf("SDL Error (%s): %s\n", loc, SDL_GetError());
}

SDLRenderer::SDLRenderer()
{
   int ret = SDL_Init(SDL_INIT_VIDEO);
   if (ret) {
      fprintf( stderr, "Error (SDL): could not initialise SDL: %s\n",
         SDL_GetError());
      exit(1);
   }

   _win = CreateMainWindow();

   // initialise the first available renderer
   _rend = SDL_CreateRenderer(_win, -1,
      SDL_RENDERER_ACCELERATED);
   if (!_rend) {
     print_SDL_error("SDL_CreateRenderer");
     exit(1);
   }

   // depends on renderer being initialised
   CreateFont();

   *_status = '\0';

   SDL_GetWindowSize(_win, &_rc.w, &_rc.h);

   _shipColors[0] = (SDL_Color {255, 0, 0});
   _shipColors[1] = (SDL_Color {0, 255, 0});
   _shipColors[2] = (SDL_Color {0, 0, 255});
   _shipColors[3] = (SDL_Color {128, 128, 128});

   _white  = {255, 255, 255};
   _yellow = {255, 255, 0};
}


SDLRenderer::~SDLRenderer()
{
   SDL_DestroyWindow(_win);
   SDL_DestroyRenderer(_rend);
   SDL_Quit();
}

void
SDLRenderer::Draw(GameState &gs, NonGameState &ngs)
{
   int ret = SDL_SetRenderDrawColor(_rend, 0, 0, 0 , SDL_ALPHA_OPAQUE);
   if (ret) {
     print_SDL_error("SDL_SetRenderDrawColor");
     exit(1);
   }

   // render clear actually uses the draw color
   ret = SDL_RenderClear(_rend);
   if (ret) {
     print_SDL_error("SDL_RenderClear");
   }

   for (int i = 0; i < gs._num_ships; i++) {
      SDL_Color color = _shipColors[i];
      int ret = SDL_SetRenderDrawColor(_rend, color.r, color.g, color.b,
           SDL_ALPHA_OPAQUE);
      if (ret) {
        print_SDL_error("SDL_SetRenderDrawColor shipcolor");
      }

      DrawShip(i, gs);
      DrawConnectState(gs._ships[i], ngs.players[i], &_shipColors[i]);
   }

   SDL_Rect dst;
   // more or less centered
   dst.x = (_rc.w / 2) - 80;
   dst.y = _rc.h - 32;
   DrawText(_status, &dst, &_white);

   SDL_Color col = {192, 192, 192};
   RenderChecksum(40, ngs.periodic, &col);

   col = {128, 128, 128};
   RenderChecksum(56, ngs.now, &col);

   SDL_RenderPresent(_rend);
}

void
SDLRenderer::DrawText(char* text, SDL_Rect* dst, SDL_Color* color)
{
  if (!text) {
    return;
  }

  size_t text_len = strlen(text);
  if (text_len == 0) {
    return;
  }

  int ret = SDL_SetTextureColorMod(_font,
                                   color->r,
                                   color->g,
                                   color->b);
  if (ret) {
      print_SDL_error("SDL_SetTextureColorMod");
      exit(1);
  }

  SDL_Rect src;
  SDL_Rect ndst;
  for (int i = 0; i < text_len; i++) {
    Glyph glyph = glyphs_Arial[text[i]];

    src.x = glyph.x;
    src.y = glyph.y;
    src.w = glyph.width;
    src.h = glyph.height;

    ndst.w = glyph.width;
    ndst.h = glyph.height;
    ndst.x = dst->x - glyph.originX;
    ndst.y = dst->y - glyph.originY + 15;

    ret = SDL_RenderCopy(_rend, _font, &src, &ndst);
    if (ret) {
      print_SDL_error("SDL_RenderCopy");
      exit(1);
    }

    dst->x += src.w;
  }
}

void
SDLRenderer::RenderChecksum(int y, NonGameState::ChecksumInfo &info, SDL_Color* color)
{
   char checksum[128];
   sprintf(checksum, "Frame: %04d  Checksum: %08x", info.framenumber, info.checksum);

   SDL_Rect dst;
   // about centered
   dst.x = (_rc.w / 2) - 120;
   dst.y = y;
   DrawText(checksum, &dst, color);
}


void
SDLRenderer::SetStatusText(const char *text)
{
   strcpy(_status, text);
}

void
SDLRenderer::DrawShip(int which, GameState& gs)
{
    Ship* ship = gs._ships + which;
    SDL_Rect bullet;
    bullet.w = 2;
    bullet.h = 2;

    SDL_Point shape[] = {
       { SHIP_RADIUS,           0 },
       { -SHIP_RADIUS,          SHIP_WIDTH },
       { SHIP_TUCK - SHIP_RADIUS, 0 },
       { -SHIP_RADIUS,          -SHIP_WIDTH },
       { SHIP_RADIUS,           0 },
    };
    const int alignment_adjustment[] = {
       -5,
       65,
       -5,
       65,
    };
    SDL_Point text_offsets[4];
    text_offsets[0] = {gs._bounds.x + 2, gs._bounds.y + 2};
    text_offsets[1] = {gs._bounds.x + gs._bounds.w - 2, gs._bounds.y + 2};
    text_offsets[2] = {gs._bounds.x + 2, gs._bounds.y + gs._bounds.h - 20};
    text_offsets[3] = {gs._bounds.x + gs._bounds.w - 2, gs._bounds.y + gs._bounds.h - 20};

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
   SDL_RenderDrawLines(_rend, shape, ARRAY_SIZE(shape));

   int ret = SDL_SetRenderDrawColor(_rend, _yellow.r, _yellow.g, _yellow.b,
         SDL_ALPHA_OPAQUE);
   for (int i = 0; i < MAX_BULLETS; i++) {
      if (ship->bullets[i].active) {
         bullet.x = ship->bullets[i].position.x - 1;
         bullet.y = ship->bullets[i].position.y - 1;
         SDL_RenderFillRect(_rend, &bullet);
      }
   }

   sprintf(buf, "Hits: %d", ship->score);

   SDL_Rect dst;
   dst.x = text_offsets[which].x - alignment_adjustment[which];
   dst.y = text_offsets[which].y;
   DrawText(buf, &dst, &_shipColors[which]);
}

void
SDLRenderer::DrawConnectState(Ship& ship, PlayerConnectionInfo &info, SDL_Color* color)
{
   char status[64];
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
         progress = (GetCurrentTimeMS() - info.disconnect_start) * 100 / info.disconnect_timeout;
         break;
   }

   if (*status) {
       SDL_Rect dst;
       dst.x = ship.position.x - 40;
       dst.y = ship.position.y + PROGRESS_TEXT_OFFSET;
       DrawText(status, &dst, color);
   }

   if (progress >= 0) {
      SDL_Rect rc = { ship.position.x - (PROGRESS_BAR_WIDTH / 2),
                      ship.position.y + PROGRESS_BAR_TOP_OFFSET,
                      PROGRESS_BAR_WIDTH / 2,
                      PROGRESS_BAR_TOP_OFFSET + PROGRESS_BAR_HEIGHT };
      SDL_RenderDrawRect(_rend, &rc);
      rc.w = rc.x + std::min(100, progress) * PROGRESS_BAR_WIDTH / 100;
      SDL_RenderFillRect(_rend, &rc);
   }
}

int
SDLRenderer::WindowWidth()
{
   return _rc.w;
}

int
SDLRenderer::WindowHeight()
{
   return _rc.h;
}


void
SDLRenderer::CreateFont()
{
   SDL_Surface* font_surf = SDL_CreateRGBSurfaceWithFormatFrom(
       font_array,
       181,     // width
       76,      // height
       32,      // depth
       4 * 181, // pitch
        SDL_PIXELFORMAT_RGBA32
       );
   if (!font_surf) {
     print_SDL_error("create font surface");
     exit(1);
   }

   _font = SDL_CreateTextureFromSurface(_rend, font_surf);
   if(!_font) {
     print_SDL_error("create font texture");
     exit(1);
   }
}
