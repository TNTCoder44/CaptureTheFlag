#pragma once

#include "raylib.h"
#include "networking/NetworkManager.hpp"

#include <string>

constexpr const int screenWidth = 800;
constexpr const int screenHeight = 450;

class Game 
{
private:
    const std::string title = "Capture The Flag";

    bool runAsServer;
    std::string serverIP;
    std::string lastReceived;

    // Create network manager
    NetworkManager network;

public: 
    Game();
    ~Game();

    void run();

};