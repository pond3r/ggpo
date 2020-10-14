
#ifndef _CONNECTION_MANAGER_H
#define _CONNECTION_MANAGER_H

#include <map>
#include <memory>
#include <string>
#include "../../Private/types.h"

/**
* ConnectionInfo is an abstract base class for defining connections.
*
* Derived classes from this class must provide a ToString function
* that provides a useful string for logging purposes about the
* details of the given connection.
*/
class ConnectionInfo {
public:
	ConnectionInfo() {}
	virtual ~ConnectionInfo() {
	}
	virtual std::string ToString() = 0;
};

/**
* Abstract class to define a connection manager interface
*
* This is a class whos purpose is to provide an abstraction from
* underlying network system calls. It must provide a non-blocking
* upd style send recv interface. When adding a connection it should
* return a unique int ID for the connection to be referred to by in
* future interactions with the manager.
*/

class GGPOUE4_API ConnectionManager {
public:
	ConnectionManager() : _id_to_issue(0) {}

	virtual ~ConnectionManager();

	/**
	* SendTo is a sendto upd style interface
	*
	* This function is expected to function similar to a standard upd
	* socket style send.
	*/
	virtual int SendTo(const char* buffer, int len, int flags, int connection_id) = 0;

	/**
	* RecvFrom is a recvfrom upd style interface
	*
	* This function is expected to function similar to a standard upd
	* socket style recvfrom. Return values are as follows:
	* greater than 0 values indicate data length.
	* 0 indicates a disconnect.
	* -1 indicates no data or some other error.
	*/
	virtual int RecvFrom(char* buffer, int len, int flags, int* connection_id) = 0;

	/**
	* ResetManager is a reset function to clear the connection_map
	*
	* This should be called if there is a need to clear all existing
	* connections without creating a new connection manager.
	*/
	virtual int ResetManager() {
		_connection_map.clear();
		return 0;
	}

	/**
	* ToString converts relevant information to a string
	*
	* This function should convert relevant information from the
	* connection info object identified by connection_id to a
	* string. The default implementation should be valid for most
	* use cases. Overload the ToString function in the derived
	* ConnectionInfo definition.
	*/
	virtual std::string ToString(int connection_id);

	void Log(const char* fmt, ...);

protected:
	/**
	* AddConnection adds a connection to the manager and returns the ID.
	*
	* This function takes in a ConnectionInfo smartpointer to an object
	* that implicitly must be a defined type that inheriteds from
	* ConnectionInfo. Derived ConnectionManagers should define their own
	* AddConnection functions with args that provide the relevant information
	* for the specific connection desired. This function should then be called
	* to add a derived ConnectionInfo object to the _connection_map and it will
	* return the connection id. Having monotonically increasing IDs is fine
	* for this use case. It is up to the user to correctly manage IDs to
	* ensure they are only used with the ConnectionManager that issued them
	* as the same IDs will likely be valid in multiple ConnectionManagers on
	* the same process.
	*/
	int AddConnection(std::shared_ptr<ConnectionInfo> info) {
		_connection_map.insert({_id_to_issue, info});
		return _id_to_issue++;
	}

	/// The current ID value to be issued to the next connection added.
	int _id_to_issue;
	/// A map of connection IDs and smart pointers to their respective info objects.
	std::map <int, std::shared_ptr<ConnectionInfo>> _connection_map;
};

#if defined(_WINDOWS)
/// UDPConnectionManager is a windows only ip address based connection manager
class UPDInfo : public ConnectionInfo   {
public:
	UPDInfo(const char* ip_address, uint16 port);

	sockaddr_in addr;

	~UPDInfo() {
	}

	virtual std::string ToString();
};

class GGPOUE4_API UDPConnectionManager : public ConnectionManager {

public:
	UDPConnectionManager();
	virtual ~UDPConnectionManager();

	virtual int SendTo(const char* buffer, int len, int flags, int connection_id);

	virtual int RecvFrom(char* buffer, int len, int flags, int* connection_id);

	int AddConnection(const char* ip_address, uint16 port);

	void Init(uint16 port);

	int FindIDFromIP(sockaddr_in* sockaddr);

protected:
	std::shared_ptr<ConnectionInfo> BuildConnectionInfo(const char* ip_address, uint16 port);

	sockaddr_in _peer_addr;

	SOCKET _socket;

};
#endif


#endif
