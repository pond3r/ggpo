/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#ifndef _TIMESYNC_H
#define _TIMESYNC_H

#include "types.hpp"
#include "game_input.hpp"

#define FRAME_WINDOW_SIZE 40
#define MIN_UNIQUE_FRAMES 10
#define MIN_FRAME_ADVANTAGE 3
#define MAX_FRAME_ADVANTAGE 9

class TimeSync
{
public:
   TimeSync();
   virtual ~TimeSync();

   void advance_frame(GameInput &input, int advantage, int radvantage);
   int recommend_frame_wait_duration(bool require_idle_input);

protected:
   int _local[FRAME_WINDOW_SIZE];
   int _remote[FRAME_WINDOW_SIZE];
   GameInput _last_inputs[MIN_UNIQUE_FRAMES];
   int _next_prediction;
};

#endif
