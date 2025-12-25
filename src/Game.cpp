#include "Game.hpp"

#include <iostream>
#include <filesystem>
#include <memory>

#include "utils/Button.hpp"
#include "utils/Filesystem.hpp"

#include "core/Infantry.hpp"

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

void Game::startNetworking()
{
    if (runAsServer) {
        network.startServer(1234);
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
        network.startClient(serverIP, 1234);
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

    entities.push_back(new Infantry({100, 200}, 0));
    entities.push_back(new Infantry({500, 200}, 1));
    
    lastReceived = "";

    beginGame = true;
}

Game::~Game() 
{  
    for (auto& entity : entities) {
        delete entity;
    }
    entities.clear();

    network.shutdown();
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

    Vector2 posA = {100,200};

    // Main game loop
    while (!WindowShouldClose() && running)    // Detect window close button or ESC key
    {
        float dt = GetFrameTime();              // delta time that passes between the loop cycles
        mousePoint = GetMousePosition();        // current mouse pos

        bool mousePressed = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

        if (beginGame)                  // start of the game, starting screen
        {
            player1Button.update(mousePoint);
            player2Button.update(mousePoint);
            if(player1Button.isPressed())
            {
                runAsServer = true;
                startNetworking();
                beginGame = false;
            }
            else if(player2Button.isPressed())
            {
                runAsServer = false;
                startNetworking();
                beginGame = false;
            }
        }
        else
        {
            if (IsKeyDown(KEY_A))
                posA.x -= speed * dt;
            if (IsKeyDown(KEY_D))
                posA.x += speed * dt;
            if (IsKeyDown(KEY_W))
                posA.y -= speed * dt;
            if (IsKeyDown(KEY_S))
                posA.y += speed * dt;


            entities[0]->setPosition(Vector2{ posA.x,posA.y });


            // player updates
            for (auto& entity : entities)
            {
				if (entity->getHealth() <= 0)
					entities.erase(std::remove(entities.begin(), entities.end(), entity), entities.end());

                bool shotsfired = false;
                if (entity->canAttack())
                {
                    auto* ent = entity->bestEnt(entities);
                    if (ent != nullptr)
                    {
                        ent->setHealth(ent->getHealth() - entity->getDamage());
                        shotsfired = true;
                    }
                }
                entity->update(dt, shotsfired);
            }
        }

        // Poll network events
        for (int i = 0; i<5; i++) {
            auto msg = network.pollEvent();
            if (msg.has_value()) {
                Vector2Serializable opponent = Vector2Serializable::deserialize(msg.value());
            }
        }

        // Draw
        BeginDrawing();
        ClearBackground(WHITE);
        //DrawTexture(background, 0, 0, WHITE);
        //DrawFPS(10, 10);

        if (beginGame)
        {
            player1Button.draw();
            player2Button.draw();      
        }
        else
        {
            for (auto& entity : entities)
            {
                entity->draw(!runAsServer);
				DrawCircleLines(entity->getPosition().x, entity->getPosition().y, entity->getAttackRange(), RED);
            }  
        }

        EndDrawing();
    }
}