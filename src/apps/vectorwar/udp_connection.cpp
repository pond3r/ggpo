
#include <ws2tcpip.h>
#include "udp_connection.h"


SOCKET
CreateSocket(uint16_t bind_port)
{
	SOCKET s;
	sockaddr_in sin;
	uint16_t port;
	int optval = 1;

	s = socket(AF_INET, SOCK_DGRAM, 0);
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof optval);
	setsockopt(s, SOL_SOCKET, SO_DONTLINGER, (const char*)&optval, sizeof optval);

	// non-blocking...
	u_long iMode = 1;
	ioctlsocket(s, FIONBIO, &iMode);

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	for (port = bind_port; port <= bind_port; port++) {
		sin.sin_port = htons(port);
		if (bind(s, (sockaddr*)&sin, sizeof sin) != SOCKET_ERROR) {
			printf("Udp bound to port: %d.\n", port);
			return s;
		}
	}
	closesocket(s);
	return INVALID_SOCKET;
}

UdpConnection::UdpConnection(uint16_t port)
{
	socket = CreateSocket(port);
}

void UdpConnection::send_to(UdpConnection* self, const char* buffer, int len, int flags, int player_num)
{
	sockaddr_in sockaddr = self->connections[player_num].recv_addr;
	int res = sendto(self->socket, buffer, len, flags,
		(struct sockaddr*)&sockaddr,
		sizeof(sockaddr));

	if (res == SOCKET_ERROR) {
		DWORD err = WSAGetLastError();

		printf("Unknown error in sendto (erro: %d  wsaerr: %d), Connection ID: %d.\n", res, err, player_num);
	}
}

int UdpConnection::receive_from(UdpConnection* self, char* buffer, int len, int flags, int* player_num)
{
	sockaddr_in    recv_addr;
	int            recv_addr_len;
	recv_addr_len = sizeof(recv_addr);

	// >0 indicates data length.
	// 0 indicates a disconnect.
	// -1 indicates no data or some other error.
	int inlen = recvfrom(self->socket, (char*)buffer, len, flags, (struct sockaddr*)&recv_addr, &recv_addr_len);

	// Assign connection_id to the id we recieved the data from.


	// Platform specific error message handling should be done in the connection manager.
	if (inlen == -1) {
		int error = WSAGetLastError();
		if (error != WSAEWOULDBLOCK) {
			printf("recvfrom WSAGetLastError returned %d (%x).\n", error, error);
		}
	}
	else if (inlen > 0) {
		printf("test");
		for (int i = 0; i < self->connections.size(); i++)
		{
			if (self->connections[i].recv_addr.sin_addr.S_un.S_addr == recv_addr.sin_addr.S_un.S_addr
				&&
				self->connections[i].recv_addr.sin_port == recv_addr.sin_port
				) {
				*player_num = self->connections[i].player_id;

			}
		}
	}

	return inlen;
}

GGPOConnection* UdpConnection::get_ggpo_connection()
{
	GGPOConnection* con = new GGPOConnection();
	con->send_to = [](void* self, const char* buffer, int len, int flags, int player_num) { UdpConnection::send_to((UdpConnection*)self, buffer, len, flags, player_num); };
	con->receive_from = [](void* self, char* buffer, int len, int flags, int* player_num) { return UdpConnection::receive_from((UdpConnection*)self, buffer, len, flags, player_num); };
	con->instance = (void*)this;

	return con;
}

int UdpConnection::add_connection(const char* ip_address, uint16_t port)
{
	int index =(int) connections.size();
	connections.push_back(UDPConnectionData(ip_address, port, index));
	return index;
}

UDPConnectionData::UDPConnectionData(const char* ip_address, uint16_t port, int index)
{
	recv_addr.sin_family = AF_INET;
	inet_pton(AF_INET, ip_address, &recv_addr.sin_addr.s_addr);
	recv_addr.sin_port = htons(port);
	player_id = index;
}
