/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#ifndef _GGPO_UNIX_H_
#define _GGPO_UNIX_H_

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <climits>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>

#include "pevents.h"

#define DebugBreak() (raise(SIGTRAP))
#define ioctlsocket ioctl
#define closesocket close
#define WSAEWOULDBLOCK EWOULDBLOCK
#define INFINITE (-1)
#define WAIT_OBJECT_0 (0x00000000L)
#define FALSE (false)
#define MAX_PATH (4096)
#define INVALID_SOCKET ((SOCKET)(~0))
#define SOCKET_ERROR (-1)

typedef neosmart::neosmart_event_t HANDLE;
typedef uint8_t byte;
typedef int SOCKET;
typedef uint32_t DWORD;

class Platform {
public:  // types
   typedef pid_t ProcessID;
public:  // functions
   static ProcessID GetProcessID() { return getpid(); }
   static void AssertFailed(char *msg) { }
   static uint32_t GetCurrentTimeMS();
   static void SleepMS(int milliseconds);
   static void CreateDirectory(const char* pathname, const void* junk);
};

#endif
