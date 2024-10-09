#include <winsock2.h>
#include <WS2tcpip.h>
#include <atomic>

extern bool connectionSuccessfull;

class Connection
{
public:
    const char* pszHostName;
    sockaddr_in* pAddr;
    std::atomic<bool> connectionStatus;
    SOCKET sock;

public:
    Connection() : sock(INVALID_SOCKET), connectionStatus(false) {}

    void setConnectionStatus(bool status)
    {
        connectionStatus.store(status);
    }

    bool getConnectionStatus() const
    {
        return connectionStatus.load();
    }
};

int ResolveHostName(const char* pszHostName,
    sockaddr_in* pAddr);

void startConnection();

void closeConnection();