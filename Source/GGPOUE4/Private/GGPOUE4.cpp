// Copyright Epic Games, Inc. All Rights Reserved.

#include "GGPOUE4.h"

#define LOCTEXT_NAMESPACE "FGGPOUE4Module"

void FGGPOUE4Module::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FGGPOUE4Module::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FGGPOUE4Module, GGPOUE4)