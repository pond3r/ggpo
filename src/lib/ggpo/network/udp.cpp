/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#include "types.hpp"
#include "udp.hpp"

SOCKET
CreateSocket(int bind_port, int retries)
{
   SOCKET s;
   sockaddr_in sin;
   int port;
   int optval = 1;

   s = socket(AF_INET, SOCK_DGRAM, 0);
   setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char *)&optval, sizeof optval);
   setsockopt(s, SOL_SOCKET, SO_DONTLINGER, (const char *)&optval, sizeof optval);

   // non-blocking...
   u_long iMode = 1;
   ioctlsocket(s, FIONBIO, &iMode);

   sin.sin_family = AF_INET;
   sin.sin_addr.s_addr = htonl(INADDR_ANY);
   for (port = bind_port; port <= bind_port + retries; port++)
   {
      sin.sin_port = htons(port);
      if (bind(s, (sockaddr *)&sin, sizeof sin) != SOCKET_ERROR)
      {
         Log("Udp bound to port: %d.\n", port);
         return s;
      }
   }
   closesocket(s);
   return INVALID_SOCKET;
}

Udp::Udp() : _socket(INVALID_SOCKET),
             _callbacks(NULL)
{
}

Udp::~Udp(void)
{
   if (_socket != INVALID_SOCKET)
   {
      closesocket(_socket);
      _socket = INVALID_SOCKET;
   }
}

void Udp::Init(int port, Poll *poll, Callbacks *callbacks)
{
   _callbacks = callbacks;

   _poll = poll;
   _poll->RegisterLoop(this);

   Log("binding udp socket to port %d.\n", port);
   _socket = CreateSocket(port, 0);
}

void Udp::SendTo(char *buffer, int len, int flags, struct sockaddr *dst, int destlen)
{
   struct sockaddr_in *to = (struct sockaddr_in *)dst;

   int res = sendto(_socket, buffer, len, flags, dst, destlen);
   if (res == SOCKET_ERROR)
   {
      DWORD err = WSAGetLastError();
      DWORD e2 = WSAENOTSOCK;
      Log("unknown error in sendto (erro: %d  wsaerr: %d).\n", res, err);
      ASSERT(FALSE && "Unknown error in sendto");
   }
   char ipbuf[INET_ADDRSTRLEN];
   // Log("sent packet length %d to %s:%d (ret:%d).\n", len, inet_ntoa(to->sin_addr), ntohs(to->sin_port), res);
   Log("sent packet length %d to %s:%d (ret:%d).\n", len, inet_ntop(AF_INET, &to->sin_addr, ipbuf, sizeof(ipbuf)), ntohs(to->sin_port), res);
}

bool Udp::OnLoopPoll(void *cookie)
{
   uint8 recv_buf[MAX_UDP_PACKET_SIZE];
   sockaddr_in recv_addr;
   int recv_addr_len;

   for (;;)
   {
      recv_addr_len = sizeof(recv_addr);
      int len = recvfrom(_socket, (char *)recv_buf, MAX_UDP_PACKET_SIZE, 0, (struct sockaddr *)&recv_addr, &recv_addr_len);

      // TODO: handle len == 0... indicates a disconnect.

      if (len == -1)
      {
         int error = WSAGetLastError();
         if (error != WSAEWOULDBLOCK)
         {
            Log("recvfrom WSAGetLastError returned %d (%x).\n", error, error);
         }
         break;
      }
      else if (len > 0)
      {
         char ipbuf[INET_ADDRSTRLEN];
         // Log("recvfrom returned (len:%d  from:%s:%d).\n", len, inet_ntoa(recv_addr.sin_addr), ntohs(recv_addr.sin_port));
         Log("recvfrom returned (len:%d  from:%s:%d).\n", len, inet_ntop(AF_INET, &recv_addr.sin_addr, ipbuf, sizeof(ipbuf)), ntohs(recv_addr.sin_port));
         UdpMsg *msg = (UdpMsg *)recv_buf;
         _callbacks->OnMsg(recv_addr, msg, len);
      }
   }
   return true;
}

void Udp::Log(const char *fmt, ...)
{
   char buf[1024];
   size_t offset;
   va_list args;

   strcpy_s(buf, "udp | ");
   offset = strlen(buf);
   va_start(args, fmt);
   vsnprintf(buf + offset, ARRAYSIZE(buf) - offset - 1, fmt, args);
   buf[ARRAYSIZE(buf) - 1] = '\0';
   ::Log(buf);
   va_end(args);
}
