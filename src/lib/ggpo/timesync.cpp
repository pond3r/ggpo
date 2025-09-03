/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#include "timesync.h"
#include <string>
TimeSync::TimeSync()
{
   memset(_local, 0, sizeof(_local));
   memset(_remote, 0, sizeof(_remote));
}

TimeSync::~TimeSync()
{
}
void TimeSync::SetFrameDelay(int frame)
{
    _frameDelay2 = frame;
}
void
TimeSync::advance_frame(GameInput &input, float advantage, float radvantage)
{
    advantage -= _remoteFrameDelay;
    radvantage *= -1;
   // Remember the last frame and frame advantage
    _local[input.frame % ARRAY_SIZE(_local)] = advantage;
   _remote[input.frame % ARRAY_SIZE(_remote)] = radvantage;
   
  
   _avgLocal = ((nFrame * _avgLocal) + advantage) / (nFrame + 1);
   _avgRemote = ((nFrame * _avgRemote) + radvantage) / (nFrame + 1);
  
   nFrame++;   
   //Clear after first 3 seconds, as this is a bit crazy
   if (!clearedInitial && nFrame == 300)
   {
       _avgLocal = _avgRemote = 0;
       clearedInitial = true;
       nFrame = 0;
   }
}
float TimeSync::LocalAdvantage() const
{
    size_t i ;
    float advantage=0;
    for (i = 0; i < ARRAY_SIZE(_local); i++) {
        advantage += _local[i];
    }
    advantage /=(float)ARRAY_SIZE(_local);
    return (advantage);
}

float TimeSync::RemoteAdvantage() const
{
    size_t i;
    float advantage = 0;;
    for (i = 0; i < ARRAY_SIZE(_local); i++) {
        advantage += _remote[i];
    }
    advantage /= (float)ARRAY_SIZE(_local);
    return (advantage);
}
float
TimeSync::recommend_frame_wait_duration(bool )
{
   
   auto advantage = LocalAdvantage();

   auto radvantage = RemoteAdvantage();


   // See if someone should take action.  The person furthest ahead
   // needs to slow down so the other user can catch up.
   // Only do this if both clients agree on who's ahead!!
    
 
 //  if (advantage  >= radvantage) {
      
   //   return 0;
  // }
   float sleep_frames = -(((radvantage + advantage) / 2.0f));


   return sleep_frames > 0  ? (float)MIN(sleep_frames, MAX_FRAME_ADVANTAGE) : (float)MAX(sleep_frames, -MAX_FRAME_ADVANTAGE);
}
