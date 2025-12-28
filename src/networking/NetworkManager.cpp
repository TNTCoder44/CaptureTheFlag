#include "NetworkManager.hpp"
#include <iostream>
#include <cstring>

NetworkManager::NetworkManager()
    : host(nullptr), peer(nullptr), isServer(false)
{
    if (enet_initialize() != 0)
    {
        std::cerr << "ENet initialization failed!\n";
    }
}

NetworkManager::~NetworkManager()
{
    shutdown();
    enet_deinitialize();
}

bool NetworkManager::startServer(enet_uint16 port)
{
    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = port;

    host = enet_host_create(&address, 32, 2, 0, 0); // max 32 clients, 2 channels
    if (!host)
    {
        std::cerr << "Failed to create ENet server host!\n";
        return false;
    }

    isServer = true;
    std::cout << "Server started on port " << port << "\n";
    return true;
}

bool NetworkManager::startClient(const std::string &hostIP, enet_uint16 port)
{
    host = enet_host_create(nullptr, 1, 2, 0, 0); // client, 1 peer
    if (!host)
    {
        std::cerr << "Failed to create ENet client host!\n";
        return false;
    }

    ENetAddress address;
    enet_address_set_host(&address, hostIP.c_str());
    address.port = port;

    peer = enet_host_connect(host, &address, 2, 0);
    if (!peer)
    {
        std::cerr << "Failed to connect to server!\n";
        return false;
    }

    isServer = false;
    std::cout << "Connecting to server " << hostIP << ":" << port << "\n";
    return true;
}

void NetworkManager::send(const void *data, size_t size, enet_uint8 channel, enet_uint32 flags)
{
    if (!data || size == 0)
        return;

    if (isServer)
        SendToClient(data, size, channel, flags);
    else
        SendToServer(data, size, channel, flags);
}

void NetworkManager::SendToServer(const void *data, size_t size, enet_uint8 channel, enet_uint32 flags)
{
    if (!peer || !host)
        return;

    ENetPacket *packet = enet_packet_create(data, size, flags);
    enet_peer_send(peer, channel, packet);
    enet_host_flush(host);
}

void NetworkManager::SendToClient(const void *data, size_t size, enet_uint8 channel, enet_uint32 flags)
{
    if (!isServer || !host)
        return;

    for (size_t i = 0; i < host->peerCount; ++i)
    {
        ENetPeer *clientPeer = &host->peers[i];
        if (clientPeer->state == ENET_PEER_STATE_CONNECTED)
        {
            ENetPacket *packet = enet_packet_create(data, size, flags);
            enet_peer_send(clientPeer, channel, packet);
        }
    }
    enet_host_flush(host);
}

std::optional<std::vector<uint8_t>> NetworkManager::pollEvent()
{
    if (!host)
        return std::nullopt;

    ENetEvent event;
    while (enet_host_service(host, &event, 0) > 0)
    {
        switch (event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
            std::cout << (isServer ? "Client connected!" : "Connected to server!") << "\n";
            if (!isServer)
                peer = event.peer;
            break;
        case ENET_EVENT_TYPE_RECEIVE:
            // Copy the bytes before destroying the ENetPacket.
            PushPacket(event.packet->data, event.packet->dataLength);
            enet_packet_destroy(event.packet);
            break;
        case ENET_EVENT_TYPE_DISCONNECT:
            std::cout << "Peer disconnected.\n";
            break;
        default:
            break;
        }
    }

    std::lock_guard<std::mutex> lock(queueMutex);
    if (!packetQueue.empty())
    {
        std::vector<uint8_t> packet = std::move(packetQueue.front());
        packetQueue.pop();
        return packet;
    }
    return std::nullopt;
}

void NetworkManager::PushPacket(const void *data, size_t size)
{
    std::lock_guard<std::mutex> lock(queueMutex);
    const uint8_t *bytes = static_cast<const uint8_t *>(data);
    packetQueue.emplace(bytes, bytes + size);
}

void NetworkManager::shutdown()
{
    if (host)
    {
        enet_host_destroy(host);
        host = nullptr;
        peer = nullptr;
    }
}
