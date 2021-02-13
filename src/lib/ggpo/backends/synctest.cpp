/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#include "synctest.h"

SyncTestBackend::SyncTestBackend(GGPOSessionCallbacks *cb,
                                 char *gamename,
                                 int frames,
                                 int num_players) :
   _sync(NULL)
{
   _callbacks = *cb;
   _num_players = num_players;
   _check_distance = frames;
   _last_verified = 0;
   _rollingback = false;
   _running = false;
   _logfp = NULL;
   _current_input.erase();
   strcpy_s(_game, gamename);

   /*
    * Initialize the synchronziation layer
    */
   Sync::Config config = { 0 };
   config.callbacks = _callbacks;
   config.num_prediction_frames = MAX_PREDICTION_FRAMES;
   _sync.Init(config);

   /*
    * Preload the ROM
    */
   _callbacks.begin_game(gamename, _callbacks.userdata);
}

SyncTestBackend::~SyncTestBackend()
{
}

GGPOErrorCode
SyncTestBackend::DoPoll(int timeout)
{
   if (!_running) {
      GGPOEvent info;

      info.code = GGPO_EVENTCODE_RUNNING;
      _callbacks.on_event(&info, _callbacks.userdata);
      _running = true;
   }
   return GGPO_OK;
}

GGPOErrorCode
SyncTestBackend::AddPlayer(GGPOPlayer *player, GGPOPlayerHandle *handle)
{
   if (player->player_num < 1 || player->player_num > _num_players) {
      return GGPO_ERRORCODE_PLAYER_OUT_OF_RANGE;
   }
   *handle = (GGPOPlayerHandle)(player->player_num - 1);
   return GGPO_OK;
}

GGPOErrorCode
SyncTestBackend::AddLocalInput(GGPOPlayerHandle player, void *values, int size)
{
   if (!_running) {
      return GGPO_ERRORCODE_NOT_SYNCHRONIZED;
   }

   int index = (int)player;
   for (int i = 0; i < size; i++) {
      _current_input.bits[(index * size) + i] |= ((char *)values)[i];
   }
   return GGPO_OK;
}

GGPOErrorCode
SyncTestBackend::SyncInput(void *values,
                           int size,
                           int *disconnect_flags)
{
   BeginLog(false);
   if (_rollingback) {
      _last_input = _saved_frames.front().input;
   } else {
      if (_sync.GetFrameCount() == 0) {
         _sync.SaveCurrentFrame();
      }
      _last_input = _current_input;
   }
   memcpy(values, _last_input.bits, size);
   if (disconnect_flags) {
      *disconnect_flags = 0;
   }
   return GGPO_OK;
}

GGPOErrorCode
SyncTestBackend::IncrementFrame(void)
{  
   _sync.IncrementFrame();
   _current_input.erase();
   
   Log("End of frame(%d)...\n", _sync.GetFrameCount());
   EndLog();

   if (_rollingback) {
      return GGPO_OK;
   }

   int frame = _sync.GetFrameCount();
   // Hold onto the current frame in our queue of saved states.  We'll need
   // the checksum later to verify that our replay of the same frame got the
   // same results.
   SavedInfo info;
   info.frame = frame;
   info.input = _last_input;
   info.cbuf = _sync.GetLastSavedFrame().cbuf;
   info.buf = (char *)malloc(info.cbuf);
   memcpy(info.buf, _sync.GetLastSavedFrame().buf, info.cbuf);
   info.checksum = _sync.GetLastSavedFrame().checksum;
   _saved_frames.push(info);

   if (frame - _last_verified == _check_distance) {
      // We've gone far enough ahead and should now start replaying frames.
      // Load the last verified frame and set the rollback flag to true.
      _sync.LoadFrame(_last_verified);

      _rollingback = true;
      while(!_saved_frames.empty()) {
         _callbacks.advance_frame(0, _callbacks.userdata);

         // Verify that the checksumn of this frame is the same as the one in our
         // list.
         info = _saved_frames.front();
         _saved_frames.pop();

         if (info.frame != _sync.GetFrameCount()) {
            RaiseSyncError("Frame number %d does not match saved frame number %d", info.frame, frame);
         }
         int checksum = _sync.GetLastSavedFrame().checksum;
         if (info.checksum != checksum) {
            LogSaveStates(info);
            RaiseSyncError("Checksum for frame %d does not match saved (%d != %d)", frame, checksum, info.checksum);
         }
         printf("Checksum %08d for frame %d matches.\n", checksum, info.frame);
         free(info.buf);
      }
      _last_verified = frame;
      _rollingback = false;
   }

   return GGPO_OK;
}

void
SyncTestBackend::RaiseSyncError(const char *fmt, ...)
{
   char buf[1024];
   va_list args;
   va_start(args, fmt);
   vsprintf_s(buf, ARRAY_SIZE(buf), fmt, args);
   va_end(args);

   puts(buf);
   OutputDebugStringA(buf);
   EndLog();
   DebugBreak();
}

GGPOErrorCode
SyncTestBackend::Logv(char *fmt, va_list list)
{
   if (_logfp) {
      vfprintf(_logfp, fmt, list);
   }
   return GGPO_OK;
}

void
SyncTestBackend::BeginLog(int saving)
{
   EndLog();

   char filename[MAX_PATH];
   CreateDirectoryA("synclogs", NULL);
   sprintf_s(filename, ARRAY_SIZE(filename), "synclogs\\%s-%04d-%s.log",
           saving ? "state" : "log",
           _sync.GetFrameCount(),
           _rollingback ? "replay" : "original");

    fopen_s(&_logfp, filename, "w");
}

void
SyncTestBackend::EndLog()
{
   if (_logfp) {
      fprintf(_logfp, "Closing log file.\n");
      fclose(_logfp);
      _logfp = NULL;
   }
}
void
SyncTestBackend::LogSaveStates(SavedInfo &info)
{
   char filename[MAX_PATH];
   sprintf_s(filename, ARRAY_SIZE(filename), "synclogs\\state-%04d-original.log", _sync.GetFrameCount());
   _callbacks.log_game_state(filename, (unsigned char *)info.buf, info.cbuf, _callbacks.userdata);

   sprintf_s(filename, ARRAY_SIZE(filename), "synclogs\\state-%04d-replay.log", _sync.GetFrameCount());
   _callbacks.log_game_state(filename, _sync.GetLastSavedFrame().buf, _sync.GetLastSavedFrame().cbuf, _callbacks.userdata);
}
