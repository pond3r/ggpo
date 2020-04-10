#include <stdint.h>
#include <unistd.h>
#include <time.h>

uint32_t GetProcessID()
{
  return getpid();
}

uint32_t GetCurrentTimeMS()
{
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (ts.tv_sec * 1000) + (ts.tv_nsec / (1000*1000));
};
