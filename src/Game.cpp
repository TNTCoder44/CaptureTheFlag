#include "Game.hpp"
#include <iostream>

Game::Game() 
{
    runAsServer = false;   // change to true to run server mode

    if (runAsServer) {
        network.StartServer(1234);
        broadcastThread = std::thread([this]() {
            BroadcastServer(runThread); // second argument uses default
        });
        std::cout << "Server running, waiting for client...\n";
    } else {
        std::string serverIP = DiscoverServer();
        if (serverIP.empty()) {
            std::cout << "No server found on LAN.\n";
            running = false;
            return;
        }
        std::cout << "Found server at " << serverIP << "\n";
        network.StartClient(serverIP, 1234);
    }
    
    InitWindow(800, 600, runAsServer ? "Server" : "Client");
    SetTargetFPS(120);

    lastReceived = "";
}

Game::~Game() 
{
    CloseWindow();        // Close window and OpenGL context
    network.Shutdown();

    if (runAsServer) {
        runThread = false;
        broadcastThread.join();
    }
}

void Game::run() 
{
    // Main game loop
    while (!WindowShouldClose() && running)    // Detect window close button or ESC key
    {
        float dt = GetFrameTime();

        // Poll network events
        auto msg = network.PollEvent();
        if (msg.has_value()) {
            lastReceived = msg.value();
            std::cout << "Got message: " << lastReceived << "\n";
        }

        // Press SPACE to send a message
        if (IsKeyPressed(KEY_SPACE)) {
            if (runAsServer) {
                network.SendToClient("Hello from SERVER!");
            } else {
                network.SendToServer("Hello from CLIENT!");
            }
        }

        // Draw
        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawText(runAsServer ? "SERVER MODE" : "CLIENT MODE", 20, 20, 20, BLACK);
        DrawText("Press SPACE to send a message", 20, 60, 20, DARKGRAY);
        DrawText(("Last received: " + lastReceived).c_str(), 20, 100, 20, BLUE);

        EndDrawing();
    }
}