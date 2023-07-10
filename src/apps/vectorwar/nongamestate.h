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
	void OnGGPOTimeSyncEvent(float framesAhead)
	{
		auto thisAdvantage = /*(int)*/(1000.0f * framesAhead / 60.0f);// *0.5f;
		nCalls++;
		if (nCalls <= 1)
		{
			if (thisAdvantage > 0)
				Sleep((int)thisAdvantage);
			return;
		}
		 
		lastAdvantage =  (lastAdvantage * 8) / 10;
			 
		 lastAdvantage += (thisAdvantage*10)/10;
		if (lastAdvantage < 0)
		{
			m_usExtraToWait = 0;
			return;
		}
		m_usExtraToWait = (int)(lastAdvantage*1000);
		if (m_usExtraToWait)
		{
			//BusyWait(m_usExtraToWait);
			m_usExtraToWait /= m_framesToSpreadWait;
			m_WaitCount = m_framesToSpreadWait;
		}
	}
	float slowDownPC() const
	{
		return m_usExtraToWait * 100.0f / m_usPerGameLoop;
	}
	int slowdown() 
	{
		return m_WaitCount-- ? m_usExtraToWait : 0;
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
};

#endif
