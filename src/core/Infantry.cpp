#include "Infantry.hpp"

#include <iostream>
#include <stdexcept>

#include "../utils/Filesystem.hpp"
#include "../utils/ViewTransform.hpp"
#include "../utils/Math.hpp"

Infantry::Infantry(Vector2 pos, int team, Vector2 desiredPos) : Entity(pos, team)
{
    position = pos;
    this->team = team;

    isShooting = false;

	desiredPosition = desiredPos;

    health = maxHealth; // 100.f
	cooldownTimer = attackCooldown; // ready to attack

    if (team == 0)
        texture = LoadTexture(FileSystem::getPath("res/R.png").c_str());
    else if (team == 1)
        texture = LoadTexture(FileSystem::getPath("res/B.png").c_str());
    else
        throw std::runtime_error("Invalid team for Infantry entity");

    // set collider
    circle.radius = 20.f;
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

    if (!isShooting)
		isShooting = shotsFired;

	printf("health: %.2f\n", health);

    if (!isShooting)                // only move player if no shots fired; cannot move and shoot at the same time
    {
        position += computeMovement(dt);
    }
}

void Infantry::draw(bool inverted)
{
    // draw the infantry texture based on health
    //
    // health // 10 = texture number
    // 10 textures based on health -> number of soldiers per texture changes
	// 2 textures for health = 100, for healthy and injured
    //

    int textureNumber = static_cast<int>(round(health / 10.f));

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

Vector2 Infantry::computeMovement(float dt)
{
    if (desiredPosition == Vector2{-1,-1})
		return { 0.f, 0.f };

    Vector2 direction = Vector2Normalize(desiredPosition - position);

    return Vector2Scale(direction, speed * dt);
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
