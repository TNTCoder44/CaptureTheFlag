#include "Game.hpp"

#include <iostream>
#include <filesystem>
#include <memory>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

#include "utils/Math.hpp"
#include "utils/ViewTransform.hpp"
#include "utils/AudioManager.hpp"

#include "core/Infantry.hpp"
#include "core/Cavalry.hpp"
#include "core/Base.hpp"
#include "core/Artillery.hpp"

Game::Game()
{
    runAsServer = false;

    InitWindow(screenWidth, screenHeight, "Capture The Flag");

#ifdef _WIN32
    Image icon = LoadImage(FileSystem::getPath("res/utils/icon.png").c_str());
    SetWindowIcon(icon);
    UnloadImage(icon);
#endif

    SetTargetFPS(120);
    
    // Init audio
    AudioManager::getInstance().Init(true);

    // init utils
    player1Button.init(FileSystem::getPath("res/utils/player1.png").c_str(), {300, 150}, 1.1f);
    player2Button.init(FileSystem::getPath("res/utils/player2.png").c_str(), {300, 300}, 1.1f);
    restartButton.init(FileSystem::getPath("res/utils/restart.png").c_str(), {250, 500}, 1.1f);

    background = LoadTexture(FileSystem::getPath("res/utils/background.png").c_str());
    coinTexture = LoadTexture(FileSystem::getPath("res/utils/coin.png").c_str());

    // Base, position later determined
    entities.push_back(new Base({0, 0}, 0));
    entities.push_back(new Base({0, 0}, 1));

    // game variables
    lastReceived = "";

    mousePoint = {0, 0};
    dt = 0.f;
    endText = "";

    beginGame = true;
    endGame = false;
}

Game::~Game()
{
    resetNetworkingState();

    for (auto &entity : entities)
    {
        delete entity;
    }
    entities.clear();

    // network already shut down by resetNetworkingState()
    UnloadTexture(background); // Unload button texture
    UnloadTexture(coinTexture);

    AudioManager::getInstance().Shutdown();

    CloseWindow(); // Close window and OpenGL context
}

int Game::allocateEntityId(int team)
{
    // Encode team in top 8 bits to avoid collisions between client / server and the two teams
    // Low 24 bits are an increasing counter for the local player (id -> 1, 2, 3...)
    // High 8 bits are the team number. here: 0, 1
    // [ 8 bits team num ][ 24 bits local entity seq ]

    const int seq = (nextLocalEntitySeq++ & 0x00FFFFFF); // only lower 24 bits with the and
    return ((team & 0xFF) << 24) | seq; // upper 8 bits for team number - combine the bits with an or to one final int
}

void Game::stopBroadcastThread()
{
    // Stop any existing broadcaster
    runThread = false;
    if (broadcastThread.joinable())
    {
        broadcastThread.join();
    }
    runThread = true;
}

void Game::resetNetworkingState()
{
    // Ensure no background thread is using NetworkManager while reset
    stopNetworkThread();
    stopBroadcastThread();

    network.shutdown();
    clientConnected = false;

    // clean packet queues
    {
        std::lock_guard<std::mutex> lock(incomingMutex);
        std::queue<PacketData> empty;
        incomingPackets.swap(empty);
    }
    {
        std::lock_guard<std::mutex> lock(outgoingMutex);
        std::queue<PacketData> empty;
        outgoingPackets.swap(empty);
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
            // check for button press to restart
            restartButton.update(mousePoint);
            if (restartButton.isPressed())
            {
                restartGame();
            }
        }
        else if (clientConnected || true) // main game loop
        {
            // get all packets sent by server/client
            getPacketsIn();

            // update currency
            static float incomeTimer = 0.f;
            incomeTimer += dt;
            if (incomeTimer >= 2.f)
            {
                currency += income;
                incomeTimer = 0.f;
            }

            // handle mouse input for rearranging troops
            if (mousePressed)
            {
                Vector2 worldPos = ViewToWorld(mousePoint, !runAsServer);

                if (!selectedTroop)
                {
                    Entity *ent = searchForTroopAt(worldPos);
                    
                    if (ent)
                    {
                        selectedTroop = true;
                        selectedEntity = ent;
                    }
                }
                else
                {
                    PacketData pkt{};
                    pkt.type = TroopType::Change;
                    pkt.entityId = selectedEntity->getID();
                    pkt.desiredPos[0] = worldPos.x;
                    pkt.desiredPos[1] = worldPos.y;
                    selectedEntity->setDesiredPosition(worldPos);
                    sendPacket(pkt);

                    selectedTroop = false;
                    selectedEntity = nullptr;
                }
            }

            PacketData pkt{};
            pkt.type = TroopType::None;
            Vector2 pos = ViewToWorld(mousePoint, !runAsServer);

            // Input handling
            if (IsKeyDown(KEY_ONE)) // Infantry
            {
                if (currency < infantryCost)
                    goto _continue;

                currency -= infantryCost;
                const int team = runAsServer ? 0 : 1;
                const Vector2 spawnPos = runAsServer ? startPosPlayer1 : startPosPlayer2;
                Entity *ent = new Infantry(spawnPos, team, pos);
                const int id = allocateEntityId(team);
                ent->setID(id);
                entities.push_back(ent);
                pkt.entityId = id;

                pkt.type = TroopType::Infantry;
            }
            else if (IsKeyDown(KEY_TWO)) // cavalry
            {
                if (currency < cavalryCost)
                    goto _continue;
                currency -= cavalryCost;
                const int team = runAsServer ? 0 : 1;
                const Vector2 spawnPos = runAsServer ? startPosPlayer1 : startPosPlayer2;
                Entity *ent = new Cavalry(spawnPos, team, pos);
                const int id = allocateEntityId(team);
                ent->setID(id);
                entities.push_back(ent);
                pkt.entityId = id;

                pkt.type = TroopType::Cavallry;
            }
            else if (IsKeyDown(KEY_THREE)) // artillery
            {
                if (currency < artilleryCost)
                    goto _continue;
                currency -= artilleryCost;
                const int team = runAsServer ? 0 : 1;
                const Vector2 spawnPos = runAsServer ? startPosPlayer1 : startPosPlayer2;
                Entity *ent = new Artillery(spawnPos, team, pos);
                const int id = allocateEntityId(team);
                ent->setID(id);
                entities.push_back(ent);
                pkt.entityId = id;

                pkt.type = TroopType::Artillery;
            }

            if (pkt.type != TroopType::None)
            {
                pkt.desiredPos[0] = pos.x;
                pkt.desiredPos[1] = pos.y;
                sendPacket(pkt);
            }

        _continue:
            update(); // update game state; entities
        }
        else
        {
            // waiting for connection screen
            // start networking again -> try to connect again
            startNetworking(); // reset networking state gets called inside
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
            DrawText(endText.c_str(), screenWidth / 2 - MeasureText(endText.c_str(), 40) / 2, screenHeight / 2 - 80, 40, BLACK);
            DrawText("Press ESC to exit, or press the button to play again.", screenWidth / 2 - MeasureText("Press ESC to exit, or press the button to play again.", 20) / 2, screenHeight / 2 - 30, 20, BLACK);
            restartButton.draw();
        }
        else if (clientConnected || true)
        {
            // draw currency
            Vector2 currencyPos = {780.f, 40.f};
            DrawTextureEx(coinTexture, {currencyPos.x - 110.f, currencyPos.y - 20.f}, 0.f, 0.1f, WHITE);
            DrawText(std::to_string(currency).c_str(), currencyPos.x - 90.f, currencyPos.y, 25, WHITE);

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
            // waiting for connection (start or reconnect)
            DrawText(runAsServer ? "You are Player 1 (Blue)" : "You are Player 2 (Red)", 10, 10, 20, BLACK);
        }

        EndDrawing();
    }
}

void Game::update()
{
    std::unordered_map<Entity *, float> pendingDamage;
    std::unordered_set<Entity *> shooters;

    std::array<float, 2> damageDealtThisFrame{{0.f, 0.f}};

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
            const float dmg = attacker->getDamage();
            pendingDamage[target] += dmg;
            shooters.insert(attacker);

            const int team = attacker->getTeam();
            if (team == 0 || team == 1)
            {
                damageDealtThisFrame[(size_t)team] += dmg;
            }
        }
    }

    // +1 for every 20 damage dealt
    const int localTeam = runAsServer ? 0 : 1;
    damageBank[(size_t)localTeam] += damageDealtThisFrame[(size_t)localTeam];
    if (damageBank[(size_t)localTeam] >= 20.f) // at least 20 damage dealt
    {
        const int earned = (int)(damageBank[(size_t)localTeam] / 20.f);
        // could be exploited if damage dealt is extremely high
        int cappedEarned = std::min(earned, 2); // max 2 currency per frame
        currency += cappedEarned;
        damageBank[(size_t)localTeam] -= 20.f * (float)cappedEarned;
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
            endText = std::string((target->getTeam() == 0) ? "The Flag goes to Player 2!" : "The Flag goes to Player 1!");

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

            // same team collision: can overlap; code block used for other purposes
            if (a->getTeam() == b->getTeam())
            {
                continue;
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
            if ((a->getTeam() == 1 && a->getPosition().y <= 250.f) || (b->getTeam() == 1 && b->getPosition().y <= 250.f) || (a->getTeam() == 0 && a->getPosition().y >= 550.f) || (b->getTeam() == 0 && b->getPosition().y >= 550.f))
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

void Game::restartGame()
{
    // reset networking (threads + enet)
    resetNetworkingState();

    // clear all entities except bases
    for (auto it = entities.begin(); it != entities.end();)
    {
        Entity *entity = *it;
        if (!entity)
        {
            it = entities.erase(it);
            continue;
        }

        if (dynamic_cast<Base *>(entity))
        {
            // reset base health
            entity->setHealth(1000.f);
            it++;
            continue;
        }

        delete entity;
        it = entities.erase(it);
    }

    // reset game variables
    currency = 30;
    damageBank = {{0.f, 0.f}};
    endText = "";
    endGame = false;
    beginGame = true;
    dt = 0.f;
    startPos.clear();
    nextLocalEntitySeq = 1;
    lastReceived = "";
    selectedTroop = false;
    selectedEntity = nullptr;
    mousePoint = {0, 0};
    // reset to starting game screen -> reconnection of players needed
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
            Entity *ent = new Infantry(spawnPos, runAsServer ? 1 : 0, Vector2{(float)pkt.desiredPos[0], (float)pkt.desiredPos[1]});
            ent->setID(pkt.entityId);
            entities.push_back(ent);
            break;
        }
        case TroopType::Cavallry:
        {
            Vector2 spawnPos = (runAsServer) ? startPosPlayer2 : startPosPlayer1;
            Entity *ent = new Cavalry(spawnPos, runAsServer ? 1 : 0, Vector2{(float)pkt.desiredPos[0], (float)pkt.desiredPos[1]});
            ent->setID(pkt.entityId);
            entities.push_back(ent);
            break;
        }
        case TroopType::Artillery:
        {
            Vector2 spawnPos = (runAsServer) ? startPosPlayer2 : startPosPlayer1;
            Entity *ent = new Artillery(spawnPos, runAsServer ? 1 : 0, Vector2{(float)pkt.desiredPos[0], (float)pkt.desiredPos[1]});
            ent->setID(pkt.entityId);
            entities.push_back(ent);
            break;
        }
        case TroopType::Change:
        {
            auto it = std::find_if(entities.begin(), entities.end(), [&](Entity *e)
                                   { return e && e->getID() == pkt.entityId; });

            if (it != entities.end())
            {
                (*it)->setDesiredPosition(Vector2{(float)pkt.desiredPos[0], (float)pkt.desiredPos[1]});
            }
            break;
        }
        case TroopType::None:
        {
            // nothing got sent
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
        if (!runAsServer && !connectAttemptStarted)
        {
            std::string serverIP;
            while (networkThreadRunning && serverIP.empty())
            {
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
    // Make restart/reconnect safe: never reuse an active thread/host.
    resetNetworkingState();

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

Entity *Game::searchForTroopAt(Vector2 worldPos)
{
    for (auto &entity : entities)
    {
        if (!entity)
            continue;

        if (entity->getTeam() != (runAsServer ? 0 : 1)) // only own troops
            continue;

        if (dynamic_cast<Base *>(entity)) // no bases for changing position
            continue;

        Vector2 entPos = entity->getPosition();
        float radius = entity->getCircleCollider().radius;

        float dist = Vector2Distance(entPos, worldPos);
        if (dist <= radius)
        {
            // found troop
            return entity;
        }
    }

    return nullptr;
}