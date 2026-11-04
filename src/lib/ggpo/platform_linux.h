/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 *
 * POSIX platform shim.  The portable core was written against Win32 and
 * MSVC's secure CRT; this header provides the small set of names it still
 * references so the same sources build on Linux and other Unix systems.
 */

#ifndef _GGPO_LINUX_H_
#define _GGPO_LINUX_H_

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

typedef int SOCKET;
typedef uint32_t DWORD;
typedef int HANDLE;

#ifndef INVALID_SOCKET
#define INVALID_SOCKET (-1)
#endif
#ifndef SOCKET_ERROR
#define SOCKET_ERROR (-1)
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef __cdecl
#define __cdecl
#endif
#ifndef INFINITE
#define INFINITE INT_MAX
#endif

/* MSVC secure-CRT shims used by the portable core. */
#ifndef sprintf_s
#define sprintf_s snprintf
#endif
#ifndef vsprintf_s
#define vsprintf_s vsnprintf
#endif
#ifndef strcpy_s
#define strcpy_s(dst, src) strcpy((dst), (src))
#endif
#ifndef strncat_s
static inline int ggpo_strncat_s(char *dst, size_t dstsz, const char *src, size_t count) {
   size_t dl = strlen(dst);
   size_t n = strlen(src);
   if (n > count) n = count;
   if (dl >= dstsz) return -1;
   if (dl + n >= dstsz) n = dstsz - dl - 1;
   memcpy(dst + dl, src, n);
   dst[dl + n] = '\0';
   return 0;
}
#define strncat_s(dst, dstsz, src, count) ggpo_strncat_s((dst), (dstsz), (src), (count))
#endif
#ifndef DebugBreak
#define DebugBreak() ((void)0)
#endif
#ifndef MAX_PATH
#define MAX_PATH 4096
#endif
#ifndef CreateDirectoryA
#define CreateDirectoryA(path, sec) mkdir((path), 0755)
#endif
#ifndef OutputDebugStringA
#define OutputDebugStringA(msg) ((void)(msg))
#endif
#ifndef fopen_s
static inline int ggpo_fopen_s(FILE **fp, const char *path, const char *mode) {
   *fp = fopen(path, mode);
   return *fp ? 0 : -1;
}
#define fopen_s(fp, path, mode) ggpo_fopen_s((fp), (path), (mode))
#endif
#ifndef Sleep
#define Sleep(ms) usleep((useconds_t)(ms) * 1000)
#endif
#ifndef closesocket
#define closesocket close
#endif
#ifndef ioctlsocket
#define ioctlsocket(s, cmd, arg) ioctl((s), (cmd), (arg))
#endif
#ifndef WSAGetLastError
#define WSAGetLastError() (errno)
#endif
#ifndef WSAEWOULDBLOCK
#define WSAEWOULDBLOCK EWOULDBLOCK
#endif

class Platform {
public:  // types
   typedef pid_t ProcessID;

public:  // functions
   static ProcessID GetProcessID() { return getpid(); }
   static void AssertFailed(char *msg) { (void)msg; }
   static uint32_t GetCurrentTimeMS();
   static int GetConfigInt(const char *name) { (void)name; return 0; }
   static bool GetConfigBool(const char *name) {
      (void)name;
      return getenv("GGPO_LOG") != NULL;
   }
};

#endif
