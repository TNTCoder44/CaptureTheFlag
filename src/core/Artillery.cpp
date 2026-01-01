#include "Artillery.hpp"

#include <iostream>
#include <stdexcept>

#include "../utils/Filesystem.hpp"
#include "../utils/ViewTransform.hpp"
#include "../utils/Math.hpp"

Artillery::Artillery(Vector2 pos, int team, Vector2 desiredPos) : Entity(pos, team)
{
    position = pos;
    this->team = team;

    isShooting = false;

    desiredPosition = desiredPos;
    health = maxHealth;             // 100.f
    cooldownTimer = attackCooldown; // ready to attack

    if (team == 0)
    {
        textureFull = LoadTexture(FileSystem::getPath("res/artillery/blue_artilleryFull.png").c_str());
        textureShooting = LoadTexture(FileSystem::getPath("res/artillery/blue_artilleryShoot.png").c_str());
    }
    else if (team == 1)
    {
        textureFull = LoadTexture(FileSystem::getPath("res/artillery/red_artilleryFull.png").c_str());
        textureShooting = LoadTexture(FileSystem::getPath("res/artillery/red_artilleryShoot.png").c_str());
    }
    else
        throw std::runtime_error("Invalid team for Artillery entity");

    // set collider
    circle.radius = 60.f;
}

Artillery::~Artillery()
{
    UnloadTexture(textureFull);
    UnloadTexture(textureShooting);
}

bool Artillery::canAttack() const
{
    if (cooldownTimer >= attackCooldown)
        return true;
    return false;
}

void Artillery::update(float dt, bool shotsFired)
{
    if (shotsFired)
        cooldownTimer = 0.f;

    cooldownTimer += dt;

    if (!isShooting)
        isShooting = shotsFired;

    if (canAttack() && !shotsFired)
        isShooting = false;

    if (!isShooting) // only move player if no shots fired; cannot move and shoot at the same time
    {
        position += computeMovement(dt);
    }
}

void Artillery::draw(bool inverted)
{
    // draw the Artillery texture 
    Texture2D texture;

    texture = textureFull;
    // adjust texture based on if shooting
    if (isShooting)
        texture = textureShooting;

    auto scale = 0.05f;

    // if it is drawn from the client side, the texture will always be drawn inverted on the other side, no matter what team it belongs to
    if (inverted)
    {
        Vector2 viewPos = WorldToView(position, inverted);                                    // from world to view coordinates
        DrawEntityTexture(texture, viewPos, {(float)texture.width, (float)texture.height}, team == 0, scale); // flipped if other player
    }
    else
    {
        // server = world coordinates
        DrawEntityTexture(texture, position, {(float)texture.width, (float)texture.height}, team == 1, scale); // flipped if other player
    }
}

Vector2 Artillery::computeMovement(float dt)
{
    if (desiredPosition == Vector2{-1, -1})
        return {0.f, 0.f};

    Vector2 direction = Vector2Normalize(desiredPosition - position);

    return Vector2Scale(direction, speed * dt);
}

Entity *Artillery::bestEnt(const std::vector<Entity *> &entities)
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
