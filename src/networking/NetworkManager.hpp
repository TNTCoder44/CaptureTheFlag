#pragma once
#include <enet/enet.h>
#include <string>
#include <optional>
#include <queue>
#include <mutex>

class NetworkManager {
public:
    NetworkManager();
    ~NetworkManager();

    // Server/Client startup
    bool StartServer(enet_uint16 port);
    bool StartClient(const std::string& host, enet_uint16 port);

    // Send messages
    void SendToServer(const std::string& message);
    void SendToClient(const std::string& message); // only server

    // Poll for received messages
    std::optional<std::string> PollEvent();

    // Shutdown network
    void Shutdown();

private:
    ENetHost* host;
    ENetPeer* peer;
    bool isServer;

    std::queue<std::string> messageQueue;
    std::mutex queueMutex;

    void PushMessage(const std::string& message);
};
