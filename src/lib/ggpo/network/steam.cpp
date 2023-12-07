/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.

 * Modified by: JacKAsterisK
 */

#include "types.h"
#include "steam.h"

GGPOSteam::GGPOSteam() :
   _callbacks(NULL)
{
    _local_steam_id.Clear();
}

GGPOSteam::~GGPOSteam(void)
{
    _local_steam_id.Clear();
}

void
GGPOSteam::Init(Poll *poll, Callbacks *callbacks)
{
   _callbacks = callbacks;

    SteamAPI_Init();
    SteamNetworking()->AllowP2PPacketRelay(true);
   _local_steam_id = SteamUser()->GetSteamID();

   _poll = poll;
   _poll->RegisterLoop(this);
}

void
GGPOSteam::SendTo(char *buffer, int len, EP2PSend flags, CSteamID &dst)
{
    SteamNetworking()->SendP2PPacket(dst, buffer, len, flags);
}

bool
GGPOSteam::OnLoopPoll(void *cookie)
{
    ggpo::uint8 recv_buf[MAX_STEAM_PACKET_SIZE];
    uint32 msgSize;
    CSteamID steamIDRemote;

    while (SteamNetworking()->IsP2PPacketAvailable(&msgSize))
    {
        if (msgSize > MAX_STEAM_PACKET_SIZE)
        {
            Log("Dropping oversized packet\n");
            SteamNetworking()->ReadP2PPacket(recv_buf, MAX_STEAM_PACKET_SIZE, &msgSize, &steamIDRemote);
            continue;
        }

        if (!SteamNetworking()->ReadP2PPacket(recv_buf, msgSize, &msgSize, &steamIDRemote))
        {
            Log("Failed to read packet\n");
            continue;
        }

        if (steamIDRemote == _local_steam_id)
		{
			continue;
		}

        _callbacks->OnMsg(steamIDRemote, (SteamMsg *)recv_buf, msgSize);
    }

    return true;
}


void
GGPOSteam::Log(const char *fmt, ...)
{
   char buf[1024];
   size_t offset;
   va_list args;

   strcpy_s(buf, "udp | ");
   offset = strlen(buf);
   va_start(args, fmt);
   vsnprintf(buf + offset, ARRAY_SIZE(buf) - offset - 1, fmt, args);
   buf[ARRAY_SIZE(buf)-1] = '\0';
   ::Log(buf);
   va_end(args);
}
