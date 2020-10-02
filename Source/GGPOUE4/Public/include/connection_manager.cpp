
#include "types.h"
#include "connection_manager.h"


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
			Log("Udp bound to port: %d.\n", port);
			return s;
		}
	}
	closesocket(s);
	return INVALID_SOCKET;
}

std::string ConnectionManager::ToString(int connection_id) {
	ASSERT(_connection_map.count(connection_id));
	std::shared_ptr<ConnectionInfo> dest_addr = _connection_map.find(connection_id)->second;
	return dest_addr->ToString();
}

void ConnectionManager::Log(const char* fmt, ...)
{
	char buf[1024];
	size_t offset;
	va_list args;

	strcpy(buf, "connection_manager | ");
	offset = strlen(buf);
	va_start(args, fmt);
	vsnprintf(buf + offset, ARRAYSIZE(buf) - offset - 1, fmt, args);
	buf[ARRAYSIZE(buf) - 1] = '\0';
	::Log(buf);
	va_end(args);
}

UDPConnectionManager::UDPConnectionManager() : _socket(INVALID_SOCKET), _peer_addr() {}

int UDPConnectionManager::SendTo(char* buffer, int len, int flags, int connection_id) {
	ASSERT(_connection_map.count(connection_id));
	if (_connection_map.count(connection_id) == 0) {
		Log("Connection not in map Connection ID: %d).\n", connection_id);
	}

	std::shared_ptr<ConnectionInfo> dest_addr = _connection_map.find(connection_id)->second;

	if ((std::dynamic_pointer_cast <UPDInfo>(dest_addr)) == NULL) {
		Log("Cast to UPDInfo failed).\n");
		ASSERT(FALSE && "Cast to UPDInfo failed");
	}

	int res = sendto(_socket, buffer, len, flags,
		(struct sockaddr*) & ((std::dynamic_pointer_cast <UPDInfo>(dest_addr))->addr),
		sizeof(std::dynamic_pointer_cast <UPDInfo>(dest_addr)->addr));

	if (res == SOCKET_ERROR) {
		DWORD err = WSAGetLastError();
		Log("unknown error in sendto (erro: %d  wsaerr: %d), Connection ID: %d.\n", res, err, connection_id);
		ASSERT(FALSE && "Unknown error in sendto");
	}

	Log("sent packet length %d to %s (ret:%d).\n", len,
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
			Log("recvfrom WSAGetLastError returned %d (%x).\n", error, error);
		}
	}

	return inlen;
}

int UDPConnectionManager::FindIDFromIP(sockaddr_in* addr) {
	for (std::map<int, std::shared_ptr<ConnectionInfo>>::iterator it = _connection_map.begin();
		it != _connection_map.end();
		++it) {
		std::shared_ptr<ConnectionInfo> dest_addr = it->second;
		if(
			((std::dynamic_pointer_cast <UPDInfo>(dest_addr))->addr.sin_addr.S_un.S_addr == addr->sin_addr.S_un.S_addr)
			&&
			((std::dynamic_pointer_cast <UPDInfo>(dest_addr))->addr.sin_port == addr->sin_port)) {
			return it->first;
		}
	}
	return -1;
}

void UDPConnectionManager::Init(u_short port) {
	Log("binding udp socket to port %d.\n", port);
	_socket = CreateSocket(port, 0);
}

int UDPConnectionManager::AddConnection(char* ip_address, u_short port) {
	return ConnectionManager::AddConnection(BuildConnectionInfo(ip_address, port));
}

UDPConnectionManager::~UDPConnectionManager() {
	if (_socket != INVALID_SOCKET) {
		closesocket(_socket);
		_socket = INVALID_SOCKET;
	}
}

std::shared_ptr<ConnectionInfo> UDPConnectionManager::BuildConnectionInfo(char* ip_address, u_short port) {
	return std::static_pointer_cast<ConnectionInfo>(std::make_shared<UPDInfo>(ip_address, port));
}

UPDInfo::UPDInfo(char* ip_address, u_short port) {
	addr.sin_family = AF_INET;
	inet_pton(AF_INET, ip_address, &addr.sin_addr.s_addr);
	addr.sin_port = htons(port);
}

std::string UPDInfo::ToString() {
	char dst_ip[1024];
	char buffer[100];
	sprintf(buffer, "Connection: IP: %s, Port: %d",
		inet_ntop(AF_INET, (void*)&addr->sin_addr, dst_ip, ARRAY_SIZE(dst_ip)),
		ntohs(addr->sin_port));
	return std::string(buffer);

}
