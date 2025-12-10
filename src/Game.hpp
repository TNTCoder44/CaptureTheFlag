#pragma once

#ifdef WIN32
#define NOGDI
#define NOUSER
#endif

#include "raylib.h"
#include "networking/NetworkManager.hpp"

#include <string>

constexpr const int screenWidth = 800;
constexpr const int screenHeight = 450;

class Game 
{
private:
    const std::string title = "Capture The Flag";
    bool running = true;

    // Create network manager
    NetworkManager network;
    bool runAsServer;
    std::string lastReceived;

    std::atomic<bool> runThread{true};
    std::thread broadcastThread;

public: 
    Game();
    ~Game();

    void run();

};