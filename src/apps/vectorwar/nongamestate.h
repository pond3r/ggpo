#ifndef _NON_GAMESTATE_H_
#define _NON_GAMESTATE_H_

#include "ggponet.h"
#define MAX_PLAYERS     64
#include <array>
#include <chrono>
/*
 * nongamestate.h --
 *
 * These are other pieces of information not related to the state
 * of the game which are useful to carry around.  They are not
 * included in the GameState class because they specifically
 * should not be rolled back.
 */
struct LoopTimer
{
public:
	void BusyWait(int uS)
	{
		auto start = std::chrono::high_resolution_clock::now();
		while (true)
		{
			auto newtime = std::chrono::high_resolution_clock::now();
			auto frameTime = (int)std::chrono::duration_cast<std::chrono::microseconds>(newtime - start).count();
			if (frameTime >= uS)
				break;
		}
	}
	int nCalls;
	void Init(unsigned int fps, unsigned int framesToSpreadWait)
	{
		m_usPerGameLoop = 1000000 / fps;
		m_usAhead = 0;
		m_usExtraToWait = 0;
		m_framesToSpreadWait = framesToSpreadWait;
		lastAdvantage = 0.0f;
		nCalls = 0;

	}
	void OnGGPOTimeSyncEvent(float framesAhead, int nTimeSyncInterval)
	{
		if (abs(framesAhead) < 0.25f)
		{
			m_usExtraToWait = 0;
			return;
		}

		

		// This message tells us we are running ahead or behind the opponent, so we should speed up or slow down our loop a bit.
		// This message comes every nTimeSyncInterval frames. We aim to speed up/slow down 25% of reported difference by the time of the 
		// next sync message. 33% is chosen in order to avoid overcompensation - the other player will be doing 25% in the opposite direction,
		// giving 50% overall. So the gap should reduce geometrically, but not overshoot.
		// We spread the 25% wait/speedup over the next nTimeSyncInterval frames, 
		auto ticksPerFrame = 1000000 / (float)60;
		float ticksAhead = ticksPerFrame * framesAhead; // could be negative if behind
		m_usExtraToWait = (int)(ticksAhead / nTimeSyncInterval);

		// Divive by 3 for reasons described above
		m_usExtraToWait /= 4;
	}

	int slowdown() 
	{
		return m_usExtraToWait;
	}


		
	float lastAdvantage = 0.0f;
	int m_usPerGameLoop;
	int m_usAhead;
	
	int m_framesToSpreadWait;
	int m_WaitCount = 0;
	int m_usExtraToWait;
private:
};

enum PlayerConnectState {
   Connecting = 0,
   Synchronizing,
   Running,
   Disconnected,
   Disconnecting,
};

struct PlayerConnectionInfo {
   GGPOPlayerType       type;
   GGPOPlayerHandle     handle;
   PlayerConnectState   state;
   int                  connect_progress;
   int                  disconnect_timeout;
   int                  disconnect_start;
};

struct NonGameState {
   struct ChecksumInfo {
      int framenumber=0;
      uint16_t checksum=0;
   };
   LoopTimer loopTimer;
   GGPONetworkStats stats;
   void SetConnectState(GGPOPlayerHandle handle, PlayerConnectState state) {
      for (int i = 0; i < num_players; i++) {
         if (players[i].handle == handle) {
            players[i].connect_progress = 0;
            players[i].state = state;
            break;
         }
      }
   }

   void SetDisconnectTimeout(GGPOPlayerHandle handle, int when, int timeout) {
      for (int i = 0; i < num_players; i++) {
         if (players[i].handle == handle) {
            players[i].disconnect_start = when;
            players[i].disconnect_timeout = timeout;
            players[i].state = Disconnecting;
            break;
         }
      }
   }

   void SetConnectState(PlayerConnectState state) {
      for (int i = 0; i < num_players; i++) {
         players[i].state = state;
      }
   }

   void UpdateConnectProgress(GGPOPlayerHandle handle, int progress) {
      for (int i = 0; i < num_players; i++) {
         if (players[i].handle == handle) {
            players[i].connect_progress = progress;
            break;
         }
      }
   }
   std::array<int, 10> rollbacksBySize = {0,0,0,0,0,0,0,0,0,0};
   GGPOPlayerHandle     local_player_handle;
   GGPOPlayerHandle     remote_player_handle;
   int LocalPLayerNumber;
   PlayerConnectionInfo players[MAX_PLAYERS];
   int inputDelay;
   int inputDelays = 0;
   int                  num_players;
   int nRollbacks = 0;
   int nTimeSyncs = 0;
   int nonTimeSyncs = 0;
   float totalFrameDelays = 0;
   ChecksumInfo         now;
   ChecksumInfo         periodic;
   int desyncFrame = -1;
   int _networkErrorCount = 0;
};

#endif
