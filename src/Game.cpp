#include "Game.hpp"

#include <iostream>
#include <filesystem>
#include <memory>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

#include "utils/Math.hpp"
#include "utils/ViewTransform.hpp"

#include "core/Infantry.hpp"
#include "core/Cavalry.hpp"
#include "core/Base.hpp"
#include "core/Artillery.hpp"

Game::Game()
{
    printf("got here\n");
    runAsServer = false;

    InitWindow(screenWidth, screenHeight, "Capture The Flag");

#ifdef _WIN32
    Image icon = LoadImage(FileSystem::getPath("res/icon.png").c_str());
    SetWindowIcon(icon);
    UnloadImage(icon);
#endif

    SetTargetFPS(120);
    InitAudioDevice(); // Initialize audio device

    player1Button.init(FileSystem::getPath("res/player1.png").c_str(), {300, 150}, 1.1f);
    player2Button.init(FileSystem::getPath("res/player2.png").c_str(), {300, 300}, 1.1f);

    background = LoadTexture(FileSystem::getPath("res/background.png").c_str());

    // Base, position later determined
    entities.push_back(new Base({0, 0}, 0));
    entities.push_back(new Base({0, 0}, 1));

    lastReceived = "";

    mousePoint = {0, 0};
    dt = 0.f;
    endText = "";

    beginGame = true;
    endGame = false;

    // for test
    // entities.push_back(new Cavalry(startPosPlayer1, 1, {400, 200}));
    entities.push_back(new Artillery(startPosPlayer2, 1, {400, 600}));
}

Game::~Game()
{
    stopNetworkThread();

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
        if (broadcastThread.joinable())
            broadcastThread.join();
    }
}

void Game::run()
{
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
        else if (endGame)
        {
            // nothing for now
        }
        else if (clientConnected) // main game loop
        {
            // get all packets sent by server/client
            getPacketsIn();

            // Input handling
            if (IsKeyDown(KEY_A)) // change
            {
                Vector2 pos = WorldToView({400, 100}, !runAsServer);

                /*
                if (runAsServer)
                    entities.push_back(new Cavalry(startPosPlayer1, 0, pos));
                else
                    entities.push_back(new Cavalry(startPosPlayer2, 1, pos));
*/
                entities.push_back(new Infantry({400, 600}, 0,{400, 300}));

                PacketData pkt{};
                pkt.type = TroopType::Infantry;
                pkt.desiredPos[0] = pos.x;
                pkt.desiredPos[1] = pos.y;

                sendPacket(pkt);
            }

            update(); // update game state; entities
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
        else if (endGame)
        {
            DrawText(endText.c_str(), screenWidth / 2 - MeasureText(endText.c_str(), 40) / 2, screenHeight / 2 - 20, 40, BLACK);
        }
        else if (clientConnected)
        {
            for (auto &entity : entities)
            {
                entity->draw(!runAsServer);

                continue;

                auto pos = entity->getPosition();
                pos = WorldToView(pos, !runAsServer);
                DrawCircleLines(
                    pos.x,
                    pos.y,
                    entity->getCircleCollider().radius,
                    BLACK);
            }
        }
        else
        {
            DrawText(runAsServer ? "You are Player 1 (Blue)" : "You are Player 2 (Red)", 10, 10, 20, BLACK);
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
        if (dynamic_cast<Base *>(attacker))
            continue;

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

        if (dynamic_cast<Base *>(target) && target->getHealth() <= 0)
        {
            endGame = true;
            endText = (target->getTeam() == 0) ? "Player 2 Wins!" : "Player 1 Wins!";
            return;
        }
    }

    startPos.clear();

    // update all entities
    for (Entity *entity : entities)
    {
        if (!entity)
            continue;
        if (entity->getHealth() <= 0)
            continue;

        startPos.reserve(entities.size());
        std::cout << entity->getPosition().x << ", " << entity->getPosition().y << "\n";
        startPos.emplace(entity, entity->getPosition());
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
    bool anyCollision = false;

    for (size_t i = 0; i < entities.size(); i++)
    {
        Entity *a = entities[i];
        if (!a)
            continue;

        for (size_t j = i + 1; j < entities.size(); j++)
        {
            Entity *b = entities[j];
            if (!b)
                continue;

            // base-base collision ignored
            if (dynamic_cast<Base *>(a) && dynamic_cast<Base *>(b))
                continue;

            Vector2 delta = Vector2Subtract(a->getPosition(), b->getPosition());
            float dist = Vector2Length(delta);
            float minDist = a->getCircleCollider().radius + b->getCircleCollider().radius;

            if (dist <= 0.0f || dist >= minDist)
                continue;

            // handle entity - base collision (stop cavalry attack-move)
            if (dynamic_cast<Base *>(a) && dynamic_cast<Cavalry *>(b))
            {
                dynamic_cast<Cavalry *>(b)->setAttackMove(false);
                anyCollision = true;
                continue;
            }
            else if (dynamic_cast<Base *>(b) && dynamic_cast<Cavalry *>(a))
            {
                dynamic_cast<Cavalry *>(a)->setAttackMove(false);
                anyCollision = true;
                continue;
            }

            // same-team collision: both stay where they were at the beginning of this step
            if (a->getTeam() == b->getTeam())
            {
COLLISION:
                auto ita = startPos.find(a);
                if (ita != startPos.end())
                    a->setPosition(ita->second);
                auto itb = startPos.find(b);
                if (itb != startPos.end())
                    b->setPosition(itb->second);

                anyCollision = true;
                continue;
            }

            // cavalry collision different teams: push, only if in combat zone
            if ((a->getTeam() == 1 && a->getPosition().y <= 250.f)
                || (b->getTeam() == 1 && b->getPosition().y <= 250.f)
                || (a->getTeam() == 0 && a->getPosition().y >= 550.f)
                || (b->getTeam() == 0 && b->getPosition().y >= 550.f)
            )
                goto COLLISION;
            
            float penetration = minDist - dist;
            Vector2 normal = Vector2Scale(delta, 1.0f / dist);

            a->setPosition(Vector2Add(a->getPosition(), Vector2Scale(normal, penetration * 0.5f)));
            b->setPosition(Vector2Subtract(b->getPosition(), Vector2Scale(normal, penetration * 0.5f)));
            anyCollision = true;
        }
    }

    return anyCollision;
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

void Game::startNetworkThread()
{
    if (networkThreadRunning)
        return;

    networkThreadRunning = true;
    networkThread = std::thread([this]()
                                { networkThreadMain(); });
}

void Game::stopNetworkThread()
{
    networkThreadRunning = false;
    if (networkThread.joinable())
        networkThread.join();
}

void Game::sendPacket(const PacketData &pkt)
{
    std::lock_guard<std::mutex> lock(outgoingMutex);
    outgoingPackets.push(pkt);
}

void Game::getPacketsIn()
{
    for (;;)
    {
        PacketData pkt{};
        {
            std::lock_guard<std::mutex> lock(incomingMutex);
            if (incomingPackets.empty())
                break;
            pkt = incomingPackets.front();
            incomingPackets.pop();
        }

        // process packet (game logic, no networking calls)
        switch (pkt.type)
        {
        case TroopType::Infantry:
        {
            Vector2 spawnPos = (runAsServer) ? startPosPlayer2 : startPosPlayer1;
            entities.push_back(new Infantry(spawnPos, runAsServer ? 1 : 0, Vector2{(float)pkt.desiredPos[0], (float)pkt.desiredPos[1]}));
            break;
        }
        case TroopType::Cavallry:
        {
            Vector2 spawnPos = (runAsServer) ? startPosPlayer2 : startPosPlayer1;
            entities.push_back(new Cavalry(spawnPos, runAsServer ? 1 : 0, Vector2{ (float)pkt.desiredPos[0], (float)pkt.desiredPos[1] }));
            break;
        }
        case TroopType::Artillery:
        {
            Vector2 spawnPos = (runAsServer) ? startPosPlayer2 : startPosPlayer1;
            entities.push_back(new Artillery(spawnPos, runAsServer ? 1 : 0, Vector2{ (float)pkt.desiredPos[0], (float)pkt.desiredPos[1] }));
            break;
        }
        default:
            break;
        }
    }
}

void Game::networkThreadMain()
{
    bool connectAttemptStarted = false;

    while (networkThreadRunning)
    {
        int i = 0;
        if (!runAsServer && !connectAttemptStarted)
        {
            i++;
            printf("Discovering server... attempt %d\n", i);
            std::string serverIP;
            std::cout << serverIP << "\n";
            while (networkThreadRunning && serverIP.empty())
            {
                printf("here\n");
                std::cout << serverIP << "\n";
                serverIP = DiscoverServer(12345, 5);
            }

            if (!serverIP.empty() && networkThreadRunning)
            {
                network.startClient(serverIP, 1234);
                connectAttemptStarted = true;
            }
        }

        // send outgoing packets
        {
            std::lock_guard<std::mutex> lock(outgoingMutex);
            while (!outgoingPackets.empty())
            {
                PacketData pkt = outgoingPackets.front();
                outgoingPackets.pop();
                network.send(pkt);
            }
        }

        // Poll enet events -> incoming packets
        for (int i = 0; i < 5; i++)
        {
            auto packet = network.pollEvent();
            if (!packet.has_value())
                break;
            if (packet->size() < sizeof(PacketData))
                continue;

            PacketData pkt{};
            std::memcpy(&pkt, packet->data(), sizeof(PacketData));

            {
                std::lock_guard<std::mutex> lock(incomingMutex);
                incomingPackets.push(pkt);
            }
        }

        // update variable for main loop
        clientConnected = network.isConnected();

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void Game::startNetworking()
{
    if (runAsServer)
    {
        network.startServer(1234);
        clientConnected = false;
        broadcastThread = std::thread([this]()
                                      {
                                          BroadcastServer(runThread); // second argument uses default
                                      });
        std::cout << "Server running, waiting for client...\n";
        startNetworkThread();
    }
    else
    {
        clientConnected = false;
        std::cout << "Searching for server on LAN...\n";
        startNetworkThread();
    }
}
