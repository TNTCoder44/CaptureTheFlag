#include "Game.hpp"
#include <iostream>
#include <filesystem>

#include "utils/Button.hpp"
#include "utils/Filesystem.hpp"

#include "core/Infantrie.hpp"

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
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            goto _again;
        }
        std::cout << "Found server at " << serverIP << "\n";
        network.StartClient(serverIP, 1234);
    }
}

Game::Game() 
{
    runAsServer = false; 
    
    InitWindow(screenWidth, screenHeight, "Capture The Flag");

#ifdef _WIN32
    Image icon = LoadImage(FileSystem::getPath("res/icon.png").c_str()); 
    SetWindowIcon(icon);
    UnloadImage(icon);
#endif


    SetTargetFPS(120);
    InitAudioDevice();      // Initialize audio device
    
    background = LoadTexture(FileSystem::getPath("res/background.png").c_str());

    entities.push_back(new Infantrie({100, 200}, 0));
    entities.push_back(new Infantrie({700, 200}, 1));
    
    lastReceived = "";

    beginGame = true;
}

Game::~Game() 
{  
    for (auto& entity : entities) {
        delete entity;
    }
    entities.clear();

    network.Shutdown();
    UnloadTexture(background);  // Unload button texture
    CloseAudioDevice();     // Close audio device

    CloseWindow();        // Close window and OpenGL context

    if (runAsServer) {
        runThread = false;
        broadcastThread.join();
    }
}

void Game::run() 
{
    float speed = 200.0f;

    Vector2 mousePoint = { 0.0f, 0.0f };
    
    Button player1Button{FileSystem::getPath("res/player1.png").c_str(), {300, 150}, 1.1};
    Button player2Button{FileSystem::getPath("res/player2.png").c_str(), {300, 300}, 1.1};

    // Main game loop
    while (!WindowShouldClose() && running)    // Detect window close button or ESC key
    {
        float dt = GetFrameTime();
        mousePoint = GetMousePosition();

        bool mousePressed = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
       
        if (beginGame)
        {
            player1Button.Update(mousePoint);
            player2Button.Update(mousePoint);
            if(player1Button.isPressed())
            {
                runAsServer = true;
                StartNetworking();
                beginGame = false;
            }
            else if(player2Button.isPressed())
            {
                runAsServer = false;
                StartNetworking();
                beginGame = false;
            }
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
        ClearBackground(WHITE);
        DrawTexture(background, 0, 0, WHITE);
        //DrawFPS(10, 10);

        if (beginGame)
        {
            player1Button.Draw();
            player2Button.Draw();      
        }
        else
        {
            for (auto& entity : entities)
            {
                entity->Draw(!runAsServer);
            }  
        }

        EndDrawing();
    }
}