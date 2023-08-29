#include "crashhandler.h"
#include <stdio.h>

CrashDelegate crashCallback = nullptr;

void HandleCrash()
{
	if (crashCallback)
	{
		crashCallback();
	}
}

void InitCrashDelegate(CrashDelegate callback)
{
	crashCallback = callback;
}
