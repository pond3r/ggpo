#include <stdint.h>
#include <windows.h>

uint32_t GetProcessID()
{
  return GetCurrentProcessId();
}

uint32_t GetCurrentTimeMS()
{
  return timeGetTime();
};
