#include "Cavalry.hpp"

#include <iostream>
#include <stdexcept>

#include "../utils/Filesystem.hpp"
#include "../utils/ViewTransform.hpp"
#include "../utils/Math.hpp"

Cavalry::Cavalry(Vector2 pos, int team, Vector2 desiredPos) : Entity(pos, team)
{
    position = pos;
    this->team = team;

    isShooting = false;
    attackMove = true;

    desiredPosition = desiredPos;

    soldiersAlive = maxSoldiers; // initial number of soldiers
    spacing = 75.f;              // spacing between soldiers in formation

    health = maxHealth;             // 100.f
    cooldownTimer = attackCooldown; // ready to attack

    if (team == 0)
    {
        textureFull = LoadTexture(FileSystem::getPath("res/cavalry/blue_cavalryFull.png").c_str());
        textureInjured = LoadTexture(FileSystem::getPath("res/cavalry/blue_cavalryVer1.png").c_str());
        textureInjured2 = LoadTexture(FileSystem::getPath("res/cavalry/blue_cavalryVer2.png").c_str());
    }
    else if (team == 1)
    {
        textureFull = LoadTexture(FileSystem::getPath("res/cavalry/red_cavalryFull.png").c_str());
        textureInjured = LoadTexture(FileSystem::getPath("res/cavalry/red_cavalryVer1.png").c_str());
        textureInjured2 = LoadTexture(FileSystem::getPath("res/cavalry/red_cavalryVer2.png").c_str());
    }
    else
        throw std::runtime_error("Invalid team for Cavalry entity");

    // set collider
    circle.radius = 20.f;

    rebuildFormation();
}

Cavalry::~Cavalry()
{
    UnloadTexture(textureFull);
    UnloadTexture(textureInjured);
    UnloadTexture(textureInjured2);
}

bool Cavalry::canAttack() const
{
    if (cooldownTimer >= attackCooldown)
        return true;
    return false;
}

void Cavalry::update(float dt, bool shotsFired)
{
    if (shotsFired)
        cooldownTimer = 0.f;

    cooldownTimer += dt;

    if (!isShooting)
        isShooting = shotsFired;

    if (canAttack() && !shotsFired)
        isShooting = false;

    
    // compute movement always because cavalry can move while shooting
    if (attackMove)
        position += computeMovement(dt);
}

void Cavalry::draw(bool inverted)
{
    // draw the Cavalry texture based on health
    //
    // health // 10 = soldiers alive
    // soldier texture -> number of soldiers per drawcall changes
    // 2 textures for health = 100, for healthy and injured
    //
    Texture2D texture;

    // adjust texture based on health
    if (health == 100.f)
    {
        texture = textureFull;
    }
    else if (health >= 50.f)
    {
        texture = textureInjured;
    }
    else
    {
        texture = textureInjured2;
    }

    for (const Vector2 &offset : formationOffsets)
    {
        // if it is drawn from the client side, the texture will always be drawn inverted on the other side, no matter what team it belongs to
        if (inverted)
        {
            Vector2 viewPos = WorldToView(position + offset, inverted);                                    // from world to view coordinates
            DrawEntityTexture(texture, viewPos, {(float)soldierSize, (float)soldierSize}, team == 0, 1.f); // flipped if other player
        }
        else
        {
            // server = world coordinates
            DrawEntityTexture(texture, position + offset, {(float)soldierSize, (float)soldierSize}, team == 1, 1.f); // flipped if other player
        }
    }
}

Vector2 Cavalry::computeMovement(float dt)
{
    if (desiredPosition == Vector2{-1, -1})
        return {0.f, 0.f};

    Vector2 direction = Vector2Normalize(desiredPosition - position);

    return Vector2Scale(direction, speed * dt);
}

std::vector<Vector2> Cavalry::generateCircleFormation()
{
    int count = soldiersAlive;

    std::vector<Vector2> offsets;
    offsets.reserve(count);

    if (count == 0)
        return offsets;

    int placed = 0;
    int ring = 0;

    while (placed < count)
    {
        float radius = ring * spacing;

        int slotsInRing = (ring == 0)
                              ? 1
                              : (int)floorf((2.0f * PI * radius) / spacing);

        slotsInRing = std::max(slotsInRing, 1);

        // printf("slots %d\n", slotsInRing);

        for (int i = 0; i < slotsInRing && placed < count; i++)
        {
            float angle = (2.0f * PI * i) / slotsInRing;

            offsets.push_back({cosf(angle) * radius,
                               sinf(angle) * radius});

            placed++;
        }

        ring++;
    }

    return offsets;
}

void Cavalry::rebuildFormation()
{
    formationOffsets = generateCircleFormation();

    float maxRadius = 0.0f;
    for (const auto &offset : formationOffsets)
    {
        maxRadius = std::max(maxRadius, Vector2Length(offset));
    }

    circle.radius = maxRadius + spacing * 0.5f;
}

Entity *Cavalry::bestEnt(const std::vector<Entity *> &entities)
{
    float dist = 1000.f;
    Entity *bestEnt = (Entity *)nullptr;
    for (auto &entity : entities)
    {
        if (entity->getTeam() == team)
            continue;
        if (entity->getHealth() <= 0.f)
            continue;

        float newDist = math::DistanceEntities(entity, this);

        if (newDist < dist)
        {
            dist = newDist;
            if (newDist <= attackRange)
                bestEnt = entity;
        }
    }

    if (bestEnt != nullptr)
        return bestEnt;
    return nullptr;
}
