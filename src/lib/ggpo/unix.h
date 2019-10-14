#ifndef _UNIX_H
#define _UNIX_H

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdarg.h>
#include <climits>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>

#include "pevents.h"

#define GetCurrentProcessId getpid
#define ARRAYSIZE(a) \
  ((sizeof(a) / sizeof(*(a))) / \
  static_cast<size_t>(!(sizeof(a) % sizeof(*(a)))))
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

uint32_t timeGetTime();
void Sleep(int milliseconds);
void CreateDirectoryA(const char* pathname, const void* junk);

#endif

