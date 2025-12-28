#pragma once
#include <string>
#include <optional>
#include <queue>
#include <mutex>
#include <enet/enet.h>
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif


class NetworkManager {
public:
    NetworkManager();
    ~NetworkManager();

    // Server/Client startup
    bool startServer(enet_uint16 port);
    bool startClient(const std::string& host, enet_uint16 port);

    inline void send(void* packet) {
        if (isServer) {
            SendToClient(packet);
        } else {
            SendToServer(packet);
        }
    }

    // Poll for received packets
    std::optional<void*> pollEvent();

    // Shutdown network
    void shutdown();

private:
    ENetHost* host;
    ENetPeer* peer;
    bool isServer;

    std::queue<void*> packetQueue;
    std::mutex queueMutex;

    // Send packets
    void SendToServer(void* packet);
    void SendToClient(void* packet);

    void PushPacket(void* packet);
};

inline void BroadcastServer(std::atomic<bool>& running, uint16_t broadcastPort = 12345) {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);
#endif

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { std::cerr << "Socket creation failed!\n"; return; }

    int broadcastEnable = 1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char*)&broadcastEnable, sizeof(broadcastEnable));

    sockaddr_in broadcastAddr{};
    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_port = htons(broadcastPort);
    broadcastAddr.sin_addr.s_addr = inet_addr("255.255.255.255");

    const char* message = "PLAYER1_HERE";

    while (running) {
        sendto(sock, message, strlen(message), 0, (sockaddr*)&broadcastAddr, sizeof(broadcastAddr));
        std::this_thread::sleep_for(std::chrono::seconds(1)); // broadcast every second
    }

#ifdef _WIN32
    closesocket(sock);
    WSACleanup();
#else
    close(sock);
#endif
}

inline std::string DiscoverServer(uint16_t broadcastPort = 12345, int timeoutSeconds = 5) {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);
#endif

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { std::cerr << "Socket creation failed!\n"; return ""; }

    sockaddr_in listenAddr{};
    listenAddr.sin_family = AF_INET;
    listenAddr.sin_port = htons(broadcastPort);
    listenAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (sockaddr*)&listenAddr, sizeof(listenAddr)) < 0) {
        std::cerr << "Bind failed!\n"; return "";
    }

    fd_set readfds;
    timeval tv{};
    tv.tv_sec = timeoutSeconds;
    tv.tv_usec = 0;

#ifdef _WIN32
    typedef int socklen_t;
#endif

    char buffer[1024];
    sockaddr_in senderAddr{};
    socklen_t addrLen = sizeof(senderAddr);

    FD_ZERO(&readfds);
    FD_SET(sock, &readfds);

    int ret = select(sock+1, &readfds, nullptr, nullptr, &tv);
    if (ret > 0 && FD_ISSET(sock, &readfds)) {
        int bytes = recvfrom(sock, buffer, sizeof(buffer)-1, 0, (sockaddr*)&senderAddr, &addrLen);
        if (bytes > 0) {
            buffer[bytes] = 0;
            if (strcmp(buffer, "PLAYER1_HERE") == 0) {
                std::string serverIP = inet_ntoa(senderAddr.sin_addr);
#ifdef _WIN32
                closesocket(sock);
                WSACleanup();
#else
                close(sock);
#endif
                return serverIP; // discovered server IP
            }
        }
    }

#ifdef _WIN32
    closesocket(sock);
    WSACleanup();
#else
    close(sock);
#endif

    return ""; // no server found
}

