/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#ifdef __linux__
#ifndef _GGPO_LINUX_H_
#define _GGPO_LINUX_H_

#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#define SOCKET_ERROR -1
#define WSAEWOULDBLOCK EWOULDBLOCK

typedef int ggpo_handle_t;
typedef int ggpo_socket_t;

class PlatformGGPO {
public:  // types
   typedef pid_t ProcessID;

public:  // functions
   static ProcessID GetProcessID() { return getpid(); }
   static void AssertFailed(char *msg) { }
   static uint32 GetCurrentTimeMS();
   static void SleepForMilliseconds(int amount) { ::usleep(amount * 1000); }
   static int GetSocketLastError() { return errno; }
};

#endif
#endif
