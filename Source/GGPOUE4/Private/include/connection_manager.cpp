
#include "include/connection_manager.h"
#include "types.h"
#include "GGPOUE4.h"


#if defined(_WINDOWS)
SOCKET
CreateSocket(uint16 bind_port, int retries)
{
	SOCKET s;
	sockaddr_in sin;
	uint16 port;
	int optval = 1;

	s = socket(AF_INET, SOCK_DGRAM, 0);
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof optval);
	setsockopt(s, SOL_SOCKET, SO_DONTLINGER, (const char*)&optval, sizeof optval);

	// non-blocking...
	u_long iMode = 1;
	ioctlsocket(s, FIONBIO, &iMode);

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	for (port = bind_port; port <= bind_port + retries; port++) {
		sin.sin_port = htons(port);
		if (bind(s, (sockaddr*)&sin, sizeof sin) != SOCKET_ERROR) {
			UE_LOG(GGPOLOG, Verbose, TEXT("Udp bound to port: %d."), port);
			return s;
		}
	}
	closesocket(s);
	return INVALID_SOCKET;
}
#endif

ConnectionManager::~ConnectionManager() {
	_connection_map.clear();
}

std::string ConnectionManager::ToString(int connection_id) {
	check(_connection_map.count(connection_id));
	std::shared_ptr<ConnectionInfo> dest_addr = _connection_map.find(connection_id)->second;
	return dest_addr->ToString();
}


#if defined(_WINDOWS)
UDPConnectionManager::UDPConnectionManager() : _socket(INVALID_SOCKET), _peer_addr() {}

int UDPConnectionManager::SendTo(const char* buffer, int len, int flags, int connection_id) {
	
	check(_connection_map.count(connection_id));
	if (_connection_map.count(connection_id) == 0) {
		UE_LOG(GGPOLOG, Warning, TEXT("Connection not in map Connection ID: %d)."), connection_id);
	}

	std::shared_ptr<ConnectionInfo> dest_addr = _connection_map.find(connection_id)->second;

	int res = sendto(_socket, buffer, len, flags,
		(struct sockaddr*) & static_cast <UPDInfo&>(*dest_addr).addr,
		sizeof(static_cast <UPDInfo&>(*dest_addr).addr));

	if (res == SOCKET_ERROR) {
		DWORD err = WSAGetLastError();

		UE_LOG(GGPOLOG, Error, TEXT("Unknown error in sendto (erro: %d  wsaerr: %d), Connection ID: %d.\n"), res, err, connection_id)		
		check(false && "Unknown error in sendto");
	}
	UE_LOG(GGPOLOG, Verbose, TEXT("Connection not in map Connection ID: %d)."), len,
		ToString(connection_id).c_str(), res);
	
	return 0;
}

int UDPConnectionManager::RecvFrom(char* buffer, int len, int flags, int* connection_id) {
	
	sockaddr_in    recv_addr;
	int            recv_addr_len;
	recv_addr_len = sizeof(recv_addr);

	// >0 indicates data length.
	// 0 indicates a disconnect.
	// -1 indicates no data or some other error.
	int inlen = recvfrom(_socket, (char*)buffer, len, flags, (struct sockaddr*) & recv_addr, &recv_addr_len);

	// Assign connection_id to the id we recieved the data from.
	*connection_id = FindIDFromIP(&recv_addr);

	// Platform specific error message handling should be done in the connection manager.
	if (inlen == -1) {
		int error = WSAGetLastError();
		if (error != WSAEWOULDBLOCK) {
			UE_LOG(GGPOLOG, Error, TEXT("recvfrom WSAGetLastError returned %d (%x)."), error, error);
		}
	}

	return inlen;
}

int UDPConnectionManager::FindIDFromIP(sockaddr_in* addr) {

	for (std::map<int, std::shared_ptr<ConnectionInfo>>::iterator it = _connection_map.begin();
		it != _connection_map.end();
		++it) {
		std::shared_ptr<ConnectionInfo> dest_addr(it->second);
		// Note: dynamic casts don't work here because UE4 doesn't allow for normal dynamic casting.
		if( static_cast <UPDInfo&>(*dest_addr).addr.sin_addr.S_un.S_addr == addr->sin_addr.S_un.S_addr
			&&
			static_cast <UPDInfo&>(*dest_addr).addr.sin_port == addr->sin_port) {
			return it->first;
		}
	}
	return -1;
}

void UDPConnectionManager::Init(uint16 port) {
	UE_LOG(GGPOLOG, Verbose, TEXT("Binding udp socket to port %d."), port);
	_socket = CreateSocket(port, 0);
}

int UDPConnectionManager::AddConnection(const char* ip_address, uint16 port) {
	return ConnectionManager::AddConnection(BuildConnectionInfo(ip_address, port));
}

UDPConnectionManager::~UDPConnectionManager() {
	if (_socket != INVALID_SOCKET) {
		closesocket(_socket);
		_socket = INVALID_SOCKET;
	}
}

std::shared_ptr<ConnectionInfo> UDPConnectionManager::BuildConnectionInfo(const char* ip_address, uint16 port) {
	return std::static_pointer_cast<ConnectionInfo>(std::make_shared<UPDInfo>(ip_address, port));
}

UPDInfo::UPDInfo(const char* ip_address, uint16 port) {
	addr.sin_family = AF_INET;
	inet_pton(AF_INET, ip_address, &addr.sin_addr.s_addr);
	addr.sin_port = htons(port);
}

std::string UPDInfo::ToString() {
	char dst_ip[1024];
	char buffer[100];
	sprintf(buffer, "Connection: IP: %s, Port: %d",
		inet_ntop(AF_INET, (void*)(&addr.sin_addr), dst_ip, ARRAY_SIZE(dst_ip)),
		ntohs(addr.sin_port));
	return std::string(buffer);

}
#endif
