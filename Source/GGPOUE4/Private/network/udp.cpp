/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#include "udp.h"
#include "types.h"

Udp::Udp() :
	_callbacks(NULL),
	_connection_manager(),
	_poll(NULL)
{
}

Udp::~Udp(void)
{
}

void
Udp::Init(Poll *poll, Callbacks *callbacks, ConnectionManager* connection_manager)
{
   _callbacks = callbacks;
   _connection_manager = connection_manager;
   _poll = poll;
   _poll->RegisterLoop(this);
}

void
Udp::SendTo(char *buffer, int len, int flags, int connection_id)
{
   _connection_manager->SendTo(buffer, len, flags, connection_id);
}

bool
Udp::OnLoopPoll(void *cookie)
{
   uint8          recv_buf[MAX_UDP_PACKET_SIZE];

   for (;;) {
	  int connection_id = -1;
	  int len = _connection_manager->RecvFrom((char*)recv_buf, MAX_UDP_PACKET_SIZE, 0, &connection_id);

      // TODO: handle len == 0... indicates a disconnect.

      if (len == -1 || connection_id == -1) {
		  if (connection_id == -1) {
	//		  Log("recvfrom returned (len:%d  from: invalid connection).\n", len);
		  }
         break;
      } else if (len > 0) {
//         Log("recvfrom returned (len:%d  from: %s).\n", len, _connection_manager->ToString(connection_id).c_str() );
         UdpMsg *msg = (UdpMsg *)recv_buf;
         _callbacks->OnMsg(connection_id, msg, len);
      }
   }
   return true;
}

void
Udp::Log(const char *fmt, ...)
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
