#ifndef _NON_GAMESTATE_H_
#define _NON_GAMESTATE_H_

#include "ggponet.h"
#define MAX_PLAYERS     64
#include <array>
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
	
	void Init(unsigned int fps, unsigned int framesToSpreadWait)
	{
		m_usPerGameLoop = 1000000 / fps;
		m_usAhead = 0;
		m_usExtraToWait = 0;
		m_framesToSpreadWait = framesToSpreadWait;
		lastAdvantage = 0.0f;

	}
	void OnGGPOTimeSyncEvent(float framesAhead)
	{
		lastAdvantage = /*(int)*/(1000.0f * framesAhead / 60.0f);

		m_usExtraToWait = (int)(lastAdvantage*1000);
		if (m_usExtraToWait)
		{
			m_usExtraToWait /= m_framesToSpreadWait;
			m_WaitCount = m_framesToSpreadWait;
		}
		/*if (framesAhead < 0.5f)
			m_usExtraToWait = 0;
		else 
			m_usExtraToWait = (int)(framesAhead*800);
	
		m_usExtraToWait = min(3000, m_usExtraToWait);*/
		////const int framesToSpreadover = 150;
		//m_usAhead = (int)((framesAhead)*(float)m_usPerGameLoop); 
		//m_usExtraToWait = max(1, m_usAhead / m_framesToSpreadWait);
		//m_WaitCount = m_framesToSpreadWait;
		//char str[256];
		//sprintf_s<256>(str, "We are %.2f frames (%dus) ahead and will wait an extra %dus per frame for %d frames\n", framesAhead, m_usAhead, m_usExtraToWait, m_framesToSpreadWait);
		//OutputDebugStringA(str);
	}
	float slowDownPC() const
	{
		return m_usExtraToWait * 100.0f / m_usPerGameLoop;
	}
	unsigned int slowdown() const
	{
		return m_WaitCount ? m_usExtraToWait : 0;
	}

	// Call every loop, to get the amount of time the current iteration of gameloop should take
	unsigned int usToWaitThisLoop()
	{
		auto timetoWait = m_usPerGameLoop;
		if (m_WaitCount) {
			timetoWait += m_usExtraToWait;
			m_WaitCount--;
			if (!m_WaitCount)
				m_usExtraToWait = 0;
		}
		return  timetoWait;
	}
		
	float lastAdvantage = 0.0f;
	unsigned int m_usPerGameLoop;
	int m_usAhead;
	unsigned int m_usExtraToWait;
	unsigned int m_framesToSpreadWait;
	unsigned int m_WaitCount = 0;
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
      int checksum=0;
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
};

#endif
