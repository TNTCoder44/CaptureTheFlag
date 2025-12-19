#include "Game.hpp"
#include <iostream>
#include <filesystem>

#include "utils/Button.hpp"
#include "utils/Filesystem.hpp"

namespace fs = std::filesystem;

struct Vector2Serializable {
    float x, y;
    std::string serialize() { return std::to_string(x) + "," + std::to_string(y); }
    static Vector2Serializable deserialize(const std::string &s) {
        Vector2Serializable v;
        std::stringstream ss(s);
        std::string item;
        getline(ss, item, ','); v.x = std::stof(item);
        getline(ss, item, ','); v.y = std::stof(item);
        return v;
    }
};

void Game::StartNetworking()
{
    if (runAsServer) {
        network.StartServer(1234);
        broadcastThread = std::thread([this]() {
            BroadcastServer(runThread); // second argument uses default
        });
        std::cout << "Server running, waiting for client...\n";
    } else {
_again:
        std::string serverIP = DiscoverServer();
        if (serverIP.empty()) {
            std::cout << "No server found on LAN.\n";
            std::this_thread::sleep_for(std::chrono::seconds(1));
            goto _again;
        }
        std::cout << "Found server at " << serverIP << "\n";
        network.StartClient(serverIP, 1234);
    }
}

Game::Game() 
{
    runAsServer = false;   // change to true to run server mode
    
    InitWindow(800, 600, "Capture The Flag");
    SetTargetFPS(120);


    
    background = LoadTexture(FileSystem::getPath("res/background.png").c_str());
    
    lastReceived = "";

    beginGame = true;
}

Game::~Game() 
{
    CloseWindow();        // Close window and OpenGL context
    network.Shutdown();
    UnloadTexture(background);  // Unload button texture

    if (runAsServer) {
        runThread = false;
        broadcastThread.join();
    }
}

void Game::run() 
{
    float speed = 200.0f;

    Vector2 mousePoint = { 0.0f, 0.0f };
    
    Button startButton{FileSystem::getPath("res/player1.png").c_str(), {300, 150}, 0.65};
    Button exitButton{FileSystem::getPath("res/player2.png").c_str(), {300, 300}, 0.65};
    
    // Main game loop
    while (!WindowShouldClose() && running)    // Detect window close button or ESC key
    {
        float dt = GetFrameTime();
        mousePoint = GetMousePosition();

        bool mousePressed = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
        startButton.Update(mousePoint);
        exitButton.Update(mousePoint);
        
        if(startButton.isPressed())
        {
            runAsServer = true;
            StartNetworking();
            beginGame = false;
        }

        if(exitButton.isPressed())
        {
            runAsServer = false;
            StartNetworking();
            beginGame = false;
        }

        // Poll network events
        for (int i = 0; i<5; i++) {
            auto msg = network.PollEvent();
            if (msg.has_value()) {
                Vector2Serializable opponent = Vector2Serializable::deserialize(msg.value());
               
            }
        }

        // Draw
        BeginDrawing();
        ClearBackground(BLACK);
        DrawTexture(background, 0, 0, WHITE);
        //DrawFPS(10, 10);

        if (beginGame)
        {
            //DrawText("Capture The Flag", 320, 200, 30, BLACK);
            startButton.Draw();
            exitButton.Draw();
        }

        EndDrawing();
    }
}