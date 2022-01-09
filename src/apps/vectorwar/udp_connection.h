#pragma once
#include <winsock.h>
#include "ggponet.h"
#include <vector>


#pragma warning(disable: 4018 4100 4127 4201 4389 4800)
struct UDPConnectionData {
	int player_id;
	sockaddr_in recv_addr;
	UDPConnectionData(const char* ip_address, uint16_t port, int index);
};

class UdpConnection
{
public:
	UdpConnection(uint16_t port);
	static void send_to(UdpConnection* self, const char* buffer, int len, int flags, int player_num);
	static int  receive_from(UdpConnection* self, char* buffer,int len, int flags, int* player_num);
	GGPOConnection* get_ggpo_connection();
	int add_connection(const char* ip_address, uint16_t port);
	std::vector<UDPConnectionData> connections;
	SOCKET socket;
};







