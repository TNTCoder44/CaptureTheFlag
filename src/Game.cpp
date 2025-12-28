#include "Game.hpp"

#include <iostream>
#include <filesystem>
#include <memory>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

#include "utils/Button.hpp"
#include "utils/Filesystem.hpp"
#include "utils/Math.hpp"
#include "utils/Packets.hpp"

#include "core/Infantry.hpp"

void Game::startNetworking()
{
    if (runAsServer)
    {
        network.startServer(1234);
        broadcastThread = std::thread([this]()
                                      {
                                          BroadcastServer(runThread); // second argument uses default
                                      });
        std::cout << "Server running, waiting for client...\n";
    }
    else
    {
    _again:
        std::string serverIP = DiscoverServer();
        if (serverIP.empty())
        {
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
    InitAudioDevice(); // Initialize audio device

    background = LoadTexture(FileSystem::getPath("res/background.png").c_str());

    // entities.push_back(new Infantry({400, 600}, 0, {400, 200}));
    // entities.push_back(new Infantry({400, 200}, 1, {400, 600}));

    lastReceived = "";

    beginGame = true;
}

Game::~Game()
{
    for (auto &entity : entities)
    {
        delete entity;
    }
    entities.clear();

    network.shutdown();
    UnloadTexture(background); // Unload button texture
    CloseAudioDevice();        // Close audio device

    CloseWindow(); // Close window and OpenGL context

    if (runAsServer)
    {
        runThread = false;
        broadcastThread.join();
    }
}

void Game::run()
{
    float speed = 200.0f;

    Vector2 mousePoint = {0.0f, 0.0f};

    Button player1Button{FileSystem::getPath("res/player1.png").c_str(), {300, 150}, 1.1};
    Button player2Button{FileSystem::getPath("res/player2.png").c_str(), {300, 300}, 1.1};

    Vector2 posA = {100, 200};
    dt = 0.f;

    bool ss = false;

    // Main game loop
    while (!WindowShouldClose() && running) // Detect window close button or ESC key
    {
        dt = GetFrameTime();             // delta time that passes between the loop cycles
        mousePoint = GetMousePosition(); // current mouse pos

        bool mousePressed = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

        if (beginGame) // start of the game, starting screen
        {
            player1Button.update(mousePoint);
            player2Button.update(mousePoint);
            if (player1Button.isPressed())
            {
                runAsServer = true;
                startNetworking();
                beginGame = false;
            }
            else if (player2Button.isPressed())
            {
                runAsServer = false;
                startNetworking();
                beginGame = false;
            }
        }
        else // main game loop
        {
            if (IsKeyDown(KEY_A) && !ss)
            {
                entities.push_back(new Infantry({400, 600}, runAsServer ? 0 : 1, {100, 100}));
                PacketData pkt{};
                pkt.type = TroopType::Infantry;
                pkt.desiredPos[0] = 100;
                pkt.desiredPos[1] = 100;

                network.send(pkt);

                ss = true;
            }

            update(); // update game state; entities
        }

        // Poll network events
        for (int i = 0; i < 5; i++)
        {
            auto packet = network.pollEvent();
            if (packet.has_value())
            {
                if (packet->size() < sizeof(PacketData))
                    continue;

                PacketData pkt{};
                std::memcpy(&pkt, packet->data(), sizeof(PacketData));

                printf("Received packet of type %d\n", static_cast<int>(pkt.type));
                // process packet
                switch (pkt.type)
                {
                case TroopType::Infantry:
                {
                    Vector2 spawnPos = (runAsServer) ? startPosPlayer1 : startPosPlayer2;
                    entities.push_back(new Infantry(spawnPos, runAsServer ? 1 : 0, Vector2{(float)pkt.desiredPos[0], (float)pkt.desiredPos[1]}));
                    break;
                }
                case TroopType::Cavallry:
                {
                    // handle cavallry spawn
                    break;
                }
                case TroopType::Artillery:
                {
                    // handle artillery spawn
                    break;
                }
                default:
                    break;
                }
            }
        }

        // Draw
        BeginDrawing();
        ClearBackground(WHITE);
        // DrawTexture(background, 0, 0, WHITE);
        // DrawFPS(10, 10);

        if (beginGame)
        {
            player1Button.draw();
            player2Button.draw();
        }
        else
        {
            for (auto &entity : entities)
            {
                entity->draw(!runAsServer);

                continue;
                DrawCircleLines(
                    entity->getPosition().x,
                    entity->getPosition().y,
                    entity->getCircleCollider().radius,
                    BLACK);
            }
        }

        EndDrawing();
    }
}

void Game::update()
{
    std::unordered_map<Entity *, float> pendingDamage;
    std::unordered_set<Entity *> shooters;

    // get all attacks this frame
    for (Entity *attacker : entities)
    {
        if (!attacker || attacker->getHealth() <= 0)
            continue;

        if (!attacker->canAttack())
            continue;

        Entity *target = attacker->bestEnt(entities);
        if (target != nullptr)
        {
            pendingDamage[target] += attacker->getDamage();
            shooters.insert(attacker);
        }
    }

    // apply all attacks
    for (auto &[target, dmg] : pendingDamage)
    {
        if (!target)
            continue;

        target->setHealth(target->getHealth() - dmg);
    }

    // update all entities
    for (Entity *entity : entities)
    {
        if (!entity)
            continue;
        if (entity->getHealth() <= 0)
            continue;
        entity->update(dt, shooters.find(entity) != shooters.end());
    }

    // remove dead entities
    for (auto it = entities.begin(); it != entities.end();)
    {
        Entity *entity = *it;
        if (!entity || entity->getHealth() <= 0)
        {
            delete entity;
            it = entities.erase(it);
            continue;
        }
        ++it;
    }

    // resolve movement collisions between entities
    resolveCollisions();
}

bool Game::resolveCollisions()
{
    for (size_t i = 0; i < entities.size(); i++)
    {
        Entity *a = entities[i];

        for (size_t j = i + 1; j < entities.size(); j++)
        {
            Entity *b = entities[j];

            Vector2 delta = Vector2Subtract(a->getPosition(), b->getPosition());
            float dist = Vector2Length(delta);
            float minDist = a->getCircleCollider().radius + b->getCircleCollider().radius;

            if (dist >= minDist || dist == 0.0f)
                return false;

            float penetration = minDist - dist;
            Vector2 normal = Vector2Scale(delta, 1.0f / dist);

            a->setPosition(Vector2Add(a->getPosition(), Vector2Scale(normal, penetration * 0.5f)));
            b->setPosition(Vector2Subtract(b->getPosition(), Vector2Scale(normal, penetration * 0.5f)));
        }
    }

    return true;
}

void Game::destroyEntityPtr(Entity *entity)
{
    auto ent = std::find(entities.begin(), entities.end(), entity);
    if (ent != entities.end())
    {
        delete *ent;
        entities.erase(ent);
    }
}

void Game::destroyEntityID(int id)
{
    auto it = std::find_if(entities.begin(), entities.end(),
                           [id](Entity *e)
                           {
                               return e && e->getID() == id;
                           });

    if (it != entities.end())
    {
        delete *it;         // delete the object
        entities.erase(it); // remove pointer from vector
    }
}
