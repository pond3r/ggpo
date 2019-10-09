/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#ifndef _TYPES_H
#define _TYPES_H


#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include "log.h"

/*
 * Keep the compiler happy
 */

/*
 * Disable specific compiler warnings
 *   4018 - '<' : signed/unsigned mismatch
 *   4100 - 'xxx' : unreferenced formal parameter
 *   4127 - conditional expression is constant
 *   4201 - nonstandard extension used : nameless struct/union
 *   4389 - '!=' : signed/unsigned mismatch
 *   4800 - 'int' : forcing value to bool 'true' or 'false' (performance warning)
 */
#pragma warning(disable: 4018 4100 4127 4201 4389 4800)


/*
 * Simple types
 */
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef char int8;
typedef short int16;
typedef int int32;

/*
 * Macros
 */
#if defined(_DEBUG)
#define BREAK  DebugBreak();
#else
#define BREAK  exit(1);
#endif


#define ASSERT(x)                                           \
   do {                                                     \
      if (!(x)) {                                           \
         char buf[1024];                                    \
         sprintf_s(buf, sizeof(buf) - 1, "Assertion: %s @ %s:%d (pid:%d)", #x, __FILE__, __LINE__, GetCurrentProcessId()); \
         Log("%s\n", buf);                                  \
         Log("\n");                                         \
         Log("\n");                                         \
         Log("\n");                                         \
         MessageBoxA(NULL, buf, "GGPO Assertion Failed", MB_OK | MB_ICONEXCLAMATION); \
         exit(0);                                           \
      }                                                     \
   } while (false)

#ifndef ARRAY_SIZE
#  define ARRAY_SIZE(a)    (sizeof(a) / sizeof((a)[0]))
#endif

#ifndef MAX_INT
#  define MAX_INT          0xEFFFFFF
#endif

#ifndef MAX
#  define MAX(x, y)        ((x) > (y) ? (x) : (y))
#endif

#endif