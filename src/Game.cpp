#include "Game.hpp"
#include <iostream>

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

    Vector2 circlePos = { 400.0f, 225.0f };
    Vector2 squarePos = { 200.0f, 225.0f };

    float speed = 200.0f;

    // Main game loop
    while (!WindowShouldClose() && running)    // Detect window close button or ESC key
    {
        float dt = GetFrameTime();

        // Press SPACE to send a message
        if (IsKeyPressed(KEY_SPACE)) {
            if (runAsServer) {
                network.Send("Hello from SERVER!");
            } else {
                network.Send("Hello from CLIENT!");
            }
        }

        if (runAsServer)
        {
            // Server controls the square
            if (IsKeyDown(KEY_LEFT))
            {
                squarePos.x -= speed * dt;
            }
            else if (IsKeyDown(KEY_RIGHT))
            {
                squarePos.x += speed * dt;
            }
            else if (IsKeyDown(KEY_UP))
            {
                squarePos.y -= speed * dt;
            }
            else if (IsKeyDown(KEY_DOWN))
            {
                squarePos.y += speed * dt;
            }
            network.Send(Vector2Serializable{squarePos.x, squarePos.y}.serialize());
        }
        else
        {
            // Client controls the circle
            if (IsKeyDown(KEY_LEFT))
            {
                circlePos.x -= speed * dt;
            }
            else if (IsKeyDown(KEY_RIGHT))
            {
                circlePos.x += speed * dt;
            }
            else if (IsKeyDown(KEY_UP))
            {
                circlePos.y -= speed * dt;
            }
            else if (IsKeyDown(KEY_DOWN))
            {
                circlePos.y += speed * dt;
            }
            network.Send(Vector2Serializable{circlePos.x, circlePos.y}.serialize());
        }

        // Poll network events
        for (int i = 0; i<5; i++) {
            auto msg = network.PollEvent();
            if (msg.has_value()) {
                Vector2Serializable opponent = Vector2Serializable::deserialize(msg.value());
                if (runAsServer) circlePos = {opponent.x, opponent.y}; // update client circle
                else squarePos = {opponent.x, opponent.y};          // update server square
            }
        }

        // Draw
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawFPS(10, 10);

        //DrawText(runAsServer ? "SERVER MODE" : "CLIENT MODE", 20, 20, 20, BLACK);
        //DrawText("Press SPACE to send a message", 20, 60, 20, DARKGRAY);
        //DrawText(("Last received: " + lastReceived).c_str(), 20, 100, 20, BLUE);
        DrawCircleV(circlePos, 20, RED);
        DrawRectangleV(squarePos, { 40, 40 }, BLUE);

        EndDrawing();
    }
}