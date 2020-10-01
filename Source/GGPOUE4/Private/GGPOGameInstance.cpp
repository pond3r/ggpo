// Copyright 2020 BwdYeti.


#include "GGPOGameInstance.h"
#include "include/ggponet.h"

void UGGPOGameInstance::CreateNetwork(int32 NumPlayers, int32 PlayerIndex, int32 LocalPort, TArray<FString> RemoteAddresses)
{
	UGGPONetwork* addresses = UGGPONetwork::CreateNetwork(
		this,
		FName(FString(TEXT("GGPONetwork"))),
		NumPlayers,
		PlayerIndex,
		LocalPort,
		RemoteAddresses);
	NetworkAddresses = addresses;
}

