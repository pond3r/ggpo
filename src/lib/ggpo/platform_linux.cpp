/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#include "platform_linux.h"

uint32_t Platform::GetCurrentTimeMS() {
   struct timespec current;
   clock_gettime(CLOCK_MONOTONIC, &current);
   return (uint32_t)((uint64_t)current.tv_sec * 1000ULL +
                   (uint64_t)current.tv_nsec / 1000000ULL);
}
