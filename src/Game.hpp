#pragma once

#ifdef WIN32
#define NOGDI
#define NOUSER
#endif

#include "raylib.h"
#include "networking/NetworkManager.hpp"

#include "core/Entity.hpp"

#include <string>
#include <queue>
#include <mutex>
#include <thread>
#include <atomic>
#include <unordered_map>

#include "utils/Packets.hpp"
#include "utils/Button.hpp"
#include "utils/Filesystem.hpp"

constexpr const int screenWidth = 800;
constexpr const int screenHeight = 800;

class Game
{
private:
    // window and rendering variables
    const std::string title = "Capture The Flag";
    bool running = true;

    Vector2 mousePoint;

    Texture2D background;

    // networking variables
    NetworkManager network;
    bool runAsServer;
    std::string lastReceived;

    std::atomic<bool> clientConnected{false};

    std::atomic<bool> networkThreadRunning{false};
    std::thread networkThread;

    std::queue<PacketData> incomingPackets;
    std::mutex incomingMutex;

    std::queue<PacketData> outgoingPackets;
    std::mutex outgoingMutex;

    std::atomic<bool> runThread{true};
    std::thread broadcastThread;

    // Entities
    std::vector<Entity *> entities;

    // game variables
    float dt; // delta time between frames

    Vector2 startPosPlayer1 = {400, 600}; // team 0
    Vector2 startPosPlayer2 = {400, 200}; // team 1

    Button player1Button;
    Button player2Button;

    bool beginGame;
    bool endGame;
    std::string endText;

    // for collision resolution between same team entities
    std::unordered_map<Entity *, Vector2> startPos;

    Texture2D coinTexture;
    int currency = 30;

    const int income = 3;

    const int infantryCost = 20;
    const int cavalryCost = 25;
    const int artilleryCost = 40;

public:
    Game();
    ~Game();

    void run();

private:
    void startNetworking();

    void startNetworkThread();
    void stopNetworkThread();
    void networkThreadMain();
    void sendPacket(const PacketData &pkt);
    void getPacketsIn();

    void update();
    bool resolveCollisions();

    void destroyEntityPtr(Entity *entity);
    void destroyEntityID(int id);
};