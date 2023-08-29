/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#ifndef _CRASH_H
#define _CRASH_H

typedef void (*CrashDelegate)();
extern void HandleCrash();
extern void InitCrashDelegate(CrashDelegate callback);

#endif
