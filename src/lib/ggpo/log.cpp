/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#include "types.h"

static FILE *logfile = NULL;

void LogFlush()
{
   if (logfile) {
      fflush(logfile);
   }
}

static char logbuf[4 * 1024 * 1024];

void Log(const char *fmt, ...)
{
   va_list args;
   va_start(args, fmt);
   Logv(fmt, args);
   va_end(args);
}

void Logv(const char *fmt, va_list args)
{
   if (!getenv("ggpo.log") || getenv("ggpo.log.ignore")) {
      return;
   }
   if (!logfile) {
      sprintf(logbuf, "log-%d.log", GetCurrentProcessId());
      logfile = fopen(logbuf, "w");
   }
   Logv(logfile, fmt, args);
}

void Logv(FILE *fp, const char *fmt, va_list args)
{
   if (getenv("ggpo.log.timestamps")) {
      static int start = 0;
      int t = 0;
      if (!start) {
         start = timeGetTime();
      } else {
         t = timeGetTime() - start;
      }
      fprintf(fp, "%d.%03d : ", t / 1000, t % 1000);
   }

   vfprintf(fp, fmt, args);
   fflush(fp);
   
   vsprintf(logbuf, fmt, args);
   //OutputDebugStringA(logbuf);
}

