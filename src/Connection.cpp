#include "connection.hpp"
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h> 
#include "renderGui.hpp"

#pragma comment(lib, "Ws2_32.lib")

Connection* CurrentConnection;



void startConnection() {
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        connectionSuccessfull = false;
        return;
    }

    sockaddr_in addrRemote = {};
    if (ResolveHostName("www.google.com", &addrRemote) != -1) {
        addrRemote.sin_port = htons(80); // Set port number (HTTP)

        // Create a socket
        SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock == INVALID_SOCKET) {
            std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
            connectionSuccessfull = false;
            WSACleanup();
            return;
        }

        // Attempt to connect to the remote server
        int result = connect(sock, (sockaddr*)&addrRemote, sizeof(addrRemote));
        if (result == SOCKET_ERROR) {
            std::cerr << "Connection failed: " << WSAGetLastError() << std::endl;
            closesocket(sock);
            WSACleanup();
            connectionSuccessfull = false;
            return;
        }

        // If connected, send a GET request
        const char* msg = "GET / HTTP/1.1\r\nHost: www.google.com\r\nConnection: close\r\n\r\n";
        int msg_len = static_cast<int>(strlen(msg));
        result = send(sock, msg, msg_len, 0);
        if (result == SOCKET_ERROR) {
            std::cerr << "Send failed: " << WSAGetLastError() << std::endl;
            closesocket(sock);
            WSACleanup();
            connectionSuccessfull = false;
            return;
        }

        // Receive the response
        char szBuffer[10000 + 1];
        result = recv(sock, szBuffer, sizeof(szBuffer) - 1, 0);
        if (result > 0) {
            // Null-terminate the received data
            szBuffer[result] = '\0';
            std::cout << "Response received:\n" << szBuffer << std::endl;
            connectionSuccessfull = true; // Mark connection as successful
        }
        else if (result == 0) {
            std::cout << "Connection closed by server." << std::endl;
            connectionSuccessfull = false;
        }
        else {
            std::cerr << "Recv failed: " << WSAGetLastError() << std::endl;
            connectionSuccessfull = false;
        }

        // Clean up
        closesocket(sock);
    }
    else {
        std::cerr << "Failed to resolve host name." << std::endl;
        connectionSuccessfull = false;
    }

    // Cleanup Winsock
    WSACleanup();
}

void disconnect() {
    if (CurrentConnection != nullptr) {
        // Close the socket if it exists
        if (CurrentConnection->sock != INVALID_SOCKET) {
            int result = closesocket(CurrentConnection->sock);
            if (result == SOCKET_ERROR) {
                std::cerr << "Socket close failed: " << WSAGetLastError() << std::endl;
            }
            else {
                std::cout << "Socket closed successfully." << std::endl;
            }
            CurrentConnection->sock = INVALID_SOCKET; // Invalidate the socket
        }

        // Clean up Winsock if necessary
        WSACleanup();
        std::cout << "Winsock cleaned up." << std::endl;

        connectionSuccessfull = false; // Mark the connection as closed
        delete CurrentConnection; // Free the connection object
        CurrentConnection = nullptr;
    }
    else {
        std::cerr << "No active connection to disconnect." << std::endl;
    }
}

void closeConnection() {
    if (CurrentConnection != nullptr) {
        // Закрытие сокета, если он существует
        if (CurrentConnection->sock != INVALID_SOCKET) {
            int result = closesocket(CurrentConnection->sock);
            if (result == SOCKET_ERROR) {
                std::cerr << "Ошибка при закрытии сокета: " << WSAGetLastError() << std::endl;
            }
            else {
                std::cout << "Сокет успешно закрыт." << std::endl;
            }
            CurrentConnection->sock = INVALID_SOCKET; // Инвалидируем сокет
        }

        // Чистка Winsock
        WSACleanup();
        std::cout << "Winsock очищен." << std::endl;

        // Удаление объекта соединения
        connectionSuccessfull = false;
        delete CurrentConnection;
        CurrentConnection = nullptr;
    }
    else {
        std::cerr << "Нет активного соединения для отключения." << std::endl;
    }
}



int ResolveHostName(const char* pszHostName, sockaddr_in* pAddr) {
    addrinfo hints = {}, * pResultList = nullptr;
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP

    int ret = getaddrinfo(pszHostName, nullptr, &hints, &pResultList);
    if (ret != 0) {
        std::cerr << "getaddrinfo failed: " << gai_strerror(ret) << std::endl;
        return -1;
    }

    if (pResultList != nullptr) {
        *pAddr = *(sockaddr_in*)(pResultList->ai_addr);
        freeaddrinfo(pResultList); // Free the address list
        return 0;
    }

    return -1;
}

