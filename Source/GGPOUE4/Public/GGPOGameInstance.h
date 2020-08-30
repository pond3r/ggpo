// Copyright 2020 BwdYeti.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GGPOGameInstance.generated.h"

// Forward declarations
class UGGPONetwork;

/**
 * 
 */
UCLASS()
class GGPOUE4_API UGGPOGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UGGPONetwork* NetworkAddresses;

    /** Creates a collection of network addresses. */
    UFUNCTION(BlueprintCallable, Category = "GGPO")
        void CreateNetwork(int32 NumPlayers, int32 PlayerIndex, int32 LocalPort, TArray<FString> RemoteAddresses);
	
};
