/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#include "ggponet.h"
#include "UObject/UObjectGlobals.h"

#define ARRAYSIZEOF(a) sizeof(a) / sizeof(a[0])

//UGGPONetworkAddress

UGGPONetworkAddress* UGGPONetworkAddress::CreateNetworkAddress(UObject* Outer, const FName Name, const FString Address)
{
    UGGPONetworkAddress* Result = NewObject<UGGPONetworkAddress>(Outer, Name);
    // Same type, apparently?
    const wchar_t* address = *Address;

    wchar_t WideIpBuffer[128];
    uint32 WideIpBufferSize = (uint32)ARRAYSIZEOF(WideIpBuffer);
    // Check and get port
    if (swscanf_s(address, L"%[^:]:%hd", WideIpBuffer, WideIpBufferSize, &Result->Port) != 2) {
        Result->bValidAddress = false;
    }
    else
    {
        // Get address
#ifdef _WIN32
		wcstombs_s(nullptr, Result->IpAddress, ARRAYSIZEOF(Result->IpAddress), WideIpBuffer, _TRUNCATE);
#else
		// FIXME: Implement this case.
#endif
	}

    return Result;
}
UGGPONetworkAddress* UGGPONetworkAddress::CreateLocalAddress(UObject* Outer, const FName Name, int32 LocalPort)
{
    UGGPONetworkAddress* Result = NewObject<UGGPONetworkAddress>(Outer, Name);

    Result->bValidAddress = true;
    Result->Port = (uint16)LocalPort;
    strcpy_s(Result->IpAddress, "127.0.0.1");

    return Result;
}

void UGGPONetworkAddress::GetIpAddress(char OutAddress[32]) const
{
    std::memcpy(OutAddress, IpAddress, sizeof(IpAddress));
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

UGGPONetwork* UGGPONetwork::CreateNetwork(UObject* Outer, const FName Name, int32 NumPlayers, int32 PlayerIndex, int32 LocalPort, TArray<FString> RemoteAddresses)
{
    UGGPONetwork* Result = NewObject<UGGPONetwork>(Outer, Name);

    Result->LocalPlayerIndex = PlayerIndex - 1;
    int32 remoteIndex = 0;
    for (int32 i = 0; i < NumPlayers; i++)
    {
        // Only the port matters for local player
        if (i == Result->LocalPlayerIndex)
        {
            // Create a GGPO Network Address and add to the addresses
            UGGPONetworkAddress* address = UGGPONetworkAddress::CreateLocalAddress(
                Outer,
                FName(FString::Printf(TEXT("P%dIPAddress"), i + 1)),
                LocalPort);
            Result->Addresses.Add(address);
        }
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
                Outer,
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
        for (int32 j = i + 1; j < Addresses.Num(); j++)
        {
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
int32 UGGPONetwork::GetLocalPort() const
{
    // Just in case
    if (LocalPlayerIndex <= -1)
        return 7000;

    return Addresses[LocalPlayerIndex]->GetPort();
}

