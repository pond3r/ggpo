/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#include "timesync.h"

TimeSync::TimeSync()
{
   memset(_local, 0, sizeof(_local));
   memset(_remote, 0, sizeof(_remote));
   _next_prediction = FRAME_WINDOW_SIZE * 3;
}

TimeSync::~TimeSync()
{
}

void
TimeSync::advance_frame(GameInput &input, int advantage, int radvantage)
{
   // Remember the last frame and frame advantage
   _last_inputs[input.frame % ARRAY_SIZE(_last_inputs)] = input;
   _local[input.frame % ARRAY_SIZE(_local)] = advantage;
   _remote[input.frame % ARRAY_SIZE(_remote)] = radvantage;
}

int
TimeSync::recommend_frame_wait_duration(bool require_idle_input)
{
   // Average our local and remote frame advantages
   int i, sum = 0;
   float advantage, radvantage;
   for (i = 0; i < ARRAY_SIZE(_local); i++) {
      sum += _local[i];
   }
   advantage = sum / (float)ARRAY_SIZE(_local);

   sum = 0;
   for (i = 0; i < ARRAY_SIZE(_remote); i++) {
      sum += _remote[i];
   }
   radvantage = sum / (float)ARRAY_SIZE(_remote);

   static int count = 0;
   count++;

   // See if someone should take action.  The person furthest ahead
   // needs to slow down so the other user can catch up.
   // Only do this if both clients agree on who's ahead!!
   if (advantage >= radvantage) {
      return 0;
   }

   // Both clients agree that we're the one ahead.  Split
   // the difference between the two to figure out how long to
   // sleep for.
   int sleep_frames = (int)(((radvantage - advantage) / 2) + 0.5);

   Log("iteration %d:  sleep frames is %d\n", count, sleep_frames);

   // Some things just aren't worth correcting for.  Make sure
   // the difference is relevant before proceeding.
   if (sleep_frames < MIN_FRAME_ADVANTAGE) {
      return 0;
   }

   // Make sure our input had been "idle enough" before recommending
   // a sleep.  This tries to make the emulator sleep while the
   // user's input isn't sweeping in arcs (e.g. fireball motions in
   // Street Fighter), which could cause the player to miss moves.
   if (require_idle_input) {
      for (i = 1; i < ARRAY_SIZE(_last_inputs); i++) {
         if (!_last_inputs[i].equal(_last_inputs[0], true)) {
            Log("iteration %d:  rejecting due to input stuff at position %d...!!!\n", count, i);
            return 0;
         }
      }
   }

   // Success!!! Recommend the number of frames to sleep and adjust
   return MIN(sleep_frames, MAX_FRAME_ADVANTAGE);
}
