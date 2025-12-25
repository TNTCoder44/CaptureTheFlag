#include "Infantry.hpp"

#include <iostream>
#include <stdexcept>

#include "../utils/Filesystem.hpp"
#include "../utils/ViewTransform.hpp"
#include "../utils/Math.hpp"

Infantry::Infantry(Vector2 pos, int team) : Entity(pos, team)
{
    position = pos;
    this->team = team;

    health = maxHealth; // 100.f
	cooldownTimer = attackCooldown; // ready to attack

    if (team == 0)
        texture = LoadTexture(FileSystem::getPath("res/R.png").c_str());
    else if (team == 1)
        texture = LoadTexture(FileSystem::getPath("res/B.png").c_str());
    else
        throw std::runtime_error("Invalid team for Infantry entity");

    // set collider
    colliderType = ColliderType::Circle;
    circle.radius = 40.0f;
    
}

Infantry::~Infantry()
{
    UnloadTexture(texture);
}

bool Infantry::canAttack() const
{
    if (cooldownTimer >= attackCooldown)
        return true;
    return false;
}

void Infantry::update(float dt, bool shotsFired)
{
    if (shotsFired)
        cooldownTimer = 0.f;

    cooldownTimer += dt;
}

void Infantry::draw(bool inverted)
{
    // if it is drawn from the client side, the texture will always be drawn inverted on the other side, no matter what team it belongs to
    if (inverted)
    {
        Vector2 viewPos = WorldToView(position, inverted); // from world to view coordinates
        DrawEntityTexture(texture, viewPos, { (float)texture.width, (float)texture.height }, team == 0, 0.05f); // flipped if other player
    }
    else
    {
        // server = world coordinates
        DrawEntityTexture(texture, position, { (float)texture.width, (float)texture.height }, team == 1, 0.05f); // flipped if other player
    }
}

Entity* Infantry::bestEnt(std::vector<Entity*> entities)
{
    float dist = 1000.f;
    Entity* bestEnt = (Entity*)nullptr;
    for (auto& entity : entities)
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
