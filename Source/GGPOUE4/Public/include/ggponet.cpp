/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#include "ggponet.h"
#include "UObject/UObjectGlobals.h"

#define ARRAYSIZE(a) sizeof(a) / sizeof(a[0])

//UGGPONetworkAddress

UGGPONetworkAddress* UGGPONetworkAddress::CreateNetworkAddress(UObject* Outer, const FName Name, const FString Address)
{
    UGGPONetworkAddress* Result = NewObject<UGGPONetworkAddress>(Outer, Name);
    // Same type, apparently?
    const wchar_t* address = *Address;

    wchar_t WideIpBuffer[128];
    uint32 WideIpBufferSize = (uint32)ARRAYSIZE(WideIpBuffer);
    // Check and get port
    if (swscanf_s(address, L"%[^:]:%hd", WideIpBuffer, WideIpBufferSize, &Result->Port) != 2) {
        Result->bValidAddress = false;
    }
    else
    {
        // Get address
        wcstombs_s(nullptr, Result->IpAddress, ARRAYSIZE(Result->IpAddress), WideIpBuffer, _TRUNCATE);
    }

    return Result;
}

void UGGPONetworkAddress::GetIpAddress(const char OutAddress[32]) const
{
    OutAddress = IpAddress;
}

bool UGGPONetworkAddress::IsValidAddress() const
{
    return bValidAddress;
}
FString UGGPONetworkAddress::GetIpAddressString() const
{
    auto address = FString(ANSI_TO_TCHAR(IpAddress));
    return address;
}
int32 UGGPONetworkAddress::GetPort() const
{
    return Port;
}

bool UGGPONetworkAddress::IsSameAddress(const UGGPONetworkAddress* Other) const
{
    if (bValidAddress != Other->bValidAddress)
        return false;
    if (!std::equal(std::begin(IpAddress), std::end(IpAddress), std::begin(Other->IpAddress)))
        return false;
    if (Port != Other->Port)
        return false;

    return true;
}

// UGGPONetwork

UGGPONetwork* UGGPONetwork::CreateNetwork(UObject* Outer, const FName Name, int32 NumPlayers, int32 PlayerIndex, TArray<FString> RemoteAddresses)
{
    UGGPONetwork* Result = NewObject<UGGPONetwork>(Outer, Name);

    Result->LocalPlayerIndex = PlayerIndex - 1;
    int32 remoteIndex = 0;
    for (int32 i = 0; i < NumPlayers; i++)
    {
        // Skip over the local player
        if (i == Result->LocalPlayerIndex)
            Result->Addresses.Add(nullptr);
        else
        {
            // If we ran out of remote addresses, clear the addresses and break
            if (remoteIndex >= RemoteAddresses.Num())
            {
                Result->Addresses.Empty();
                break;
            }

            // Create a GGPO Network Address and add to the addresses
            UGGPONetworkAddress* address = UGGPONetworkAddress::CreateNetworkAddress(
                Result,
                FName(FString::Printf(TEXT("P%dIPAddress"), i + 1)),
                RemoteAddresses[remoteIndex]);
            Result->Addresses.Add(address);
            remoteIndex++;
        }
    }

    return Result;
}

bool UGGPONetwork::AllValidAddresses() const
{
    // If there are no players, this isn't valid
    if (Addresses.Num() == 0)
        return false;

    for (int32 i = 0; i < Addresses.Num(); i++)
    {
        // Skip the local player
        if (i == LocalPlayerIndex)
            continue;

        // If an address is invalid, return false
        UGGPONetworkAddress* address = Addresses[i];
        if (!address->IsValidAddress())
            return false;
    }

    return AllUniqueAddresses();
}
bool UGGPONetwork::AllUniqueAddresses() const
{
    for (int32 i = 0; i < Addresses.Num(); i++)
    {
        // Skip the local player
        if (i == LocalPlayerIndex)
            continue;

        for (int32 j = i + 1; j < Addresses.Num(); j++)
        {
            // Skip the local player
            if (j == LocalPlayerIndex)
                continue;

            // If the address is the same, return false
            if (Addresses[i]->IsSameAddress(Addresses[j]))
                return false;
        }
    }

    return true;
}

UGGPONetworkAddress* UGGPONetwork::GetAddress(int32 Index) const
{
    if (Index < 0 || Index >= Addresses.Num())
        return nullptr;

    return Addresses[Index];
}
int32 UGGPONetwork::NumPlayers() const
{
    return Addresses.Num();
}
int32 UGGPONetwork::GetPlayerIndex() const
{
    return LocalPlayerIndex;
}

