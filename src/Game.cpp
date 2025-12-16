#include "Game.hpp"
#include <iostream>

#include "button.hpp"

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
    
    InitWindow(800, 600, runAsServer ? "Server" : "Client");
    SetTargetFPS(120);

    button = LoadTexture("res/button.png"); // Load button texture

    lastReceived = "";

    beginGame = true;
}

Game::~Game() 
{
    CloseWindow();        // Close window and OpenGL context
    network.Shutdown();
    UnloadTexture(button);  // Unload button texture

    if (runAsServer) {
        runThread = false;
        broadcastThread.join();
    }
}

void Game::run() 
{
    float speed = 200.0f;

    Vector2 mousePoint = { 0.0f, 0.0f };

    Button startButton{"res/start_button.png", {300, 150}, 0.65};
    Button exitButton{"res/exit_button.png", {300, 300}, 0.65};
    
    // Main game loop
    while (!WindowShouldClose() && running)    // Detect window close button or ESC key
    {
        float dt = GetFrameTime();
        mousePoint = GetMousePosition();

        bool mousePressed = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
       
        if(startButton.isPressed(mousePoint, mousePressed))
        {
            
        }

        if(exitButton.isPressed(mousePoint, mousePressed))
        {
            
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