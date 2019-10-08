/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 * Use of this software is prohibited unless accompanied by a license.
 */

#include "types.h"
#include "game_input.h"
#include "log.h"

void
GameInput::init(int iframe, char *ibits, int isize, int offset)
{
   ASSERT(isize);
   ASSERT(isize <= GAMEINPUT_MAX_BYTES);
   frame = iframe;
   size = isize;
   memset(bits, 0, sizeof(bits));
   if (ibits) {
      memcpy(bits + (offset * isize), ibits, isize);
   }
}

void
GameInput::init(int iframe, char *ibits, int isize)
{
   ASSERT(isize);
   ASSERT(isize <= GAMEINPUT_MAX_BYTES * GAMEINPUT_MAX_PLAYERS);
   frame = iframe;
   size = isize;
   memset(bits, 0, sizeof(bits));
   if (ibits) {
      memcpy(bits, ibits, isize);
   }
}

void
GameInput::desc(char *buf, bool show_frame) const
{
   ASSERT(size);
   int offset = 0;
   if (show_frame) {
      sprintf(buf, "(frame:%d size:%d ", frame, size);
   } else {
      sprintf(buf, "(size:%d ", size);
   }
   for (int i = 0; i < size * 8; i++) {
      char buf2[16];
      if (value(i)) {
         sprintf(buf2, "%2d ", i);
         strcat(buf, buf2);
      }
   }
   strcat(buf, ")");
}

void
GameInput::log(char *prefix, bool show_frame) const
{
	char buf[1024];
	strcpy(buf, prefix);
	desc(buf + strlen(prefix), show_frame);
   strcat(buf, "\n");
	Log(buf);
}

bool
GameInput::equal(GameInput &other, bool bitsonly)
{
   if (!bitsonly && frame != other.frame) {
      Log("frames don't match: %d, %d\n", frame, other.frame);
   }
   if (size != other.size) {
      Log("sizes don't match: %d, %d\n", size, other.size);
   }
   if (memcmp(bits, other.bits, size)) {
      Log("bits don't match\n");
   }
   ASSERT(size && other.size);
   return (bitsonly || frame == other.frame) &&
          size == other.size &&
          memcmp(bits, other.bits, size) == 0;
}

