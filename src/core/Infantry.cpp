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

    soldiersAlive = 10; // initial number of soldiers
    spacing = 15.f;     // spacing between soldiers in formation

    health = maxHealth; // 100.f
	cooldownTimer = attackCooldown; // ready to attack

    if (team == 0)
    {
        textureFull = LoadTexture(FileSystem::getPath("res/R.png").c_str());
        textureInjured = LoadTexture(FileSystem::getPath("res/R.png").c_str());
    }
    else if (team == 1)
    {
        textureFull = LoadTexture(FileSystem::getPath("res/B.png").c_str());
        textureInjured = LoadTexture(FileSystem::getPath("res/B.png").c_str());
    }
    else
        throw std::runtime_error("Invalid team for Infantry entity");

    // set collider
    circle.radius = 20.f;

    rebuildFormation();
}

Infantry::~Infantry()
{
    UnloadTexture(textureFull);
    UnloadTexture(textureInjured);
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

    if (canAttack() && !shotsFired)
        isShooting = false;

	printf("health: %.2f\n", health);

    if (!isShooting)                // only move player if no shots fired; cannot move and shoot at the same time
    {
        position += computeMovement(dt);
    }

    soldiersAlive = static_cast<int>(round(health / 10.f));
}

void Infantry::draw(bool inverted)
{
    // draw the infantry texture based on health
    //
    // health // 10 = soldiers alive
    // soldier texture -> number of soldiers per drawcall changes
	// 2 textures for health = 100, for healthy and injured
    //

    auto texture = (health == maxHealth) ? textureFull : textureInjured;

    int soldierSize = 12;

    for (const Vector2& offset : formationOffsets)
    {
        // if it is drawn from the client side, the texture will always be drawn inverted on the other side, no matter what team it belongs to
        if (inverted)
        {
            Vector2 viewPos = WorldToView(position + offset, inverted); // from world to view coordinates
            DrawEntityTexture(texture, viewPos, { (float)soldierSize, (float)soldierSize }, team == 0, 1.f); // flipped if other player
        }
        else
        {
            // server = world coordinates
            DrawEntityTexture(texture, position + offset, { (float)soldierSize, (float)soldierSize }, team == 1, 1.f); // flipped if other player
        }
    }
}

Vector2 Infantry::computeMovement(float dt)
{
    if (desiredPosition == Vector2{-1,-1})
		return { 0.f, 0.f };

    Vector2 direction = Vector2Normalize(desiredPosition - position);

    return Vector2Scale(direction, speed * dt);
}

std::vector<Vector2> Infantry::generateCircleFormation()
{
    int count = soldiersAlive;

    std::vector<Vector2> offsets;
    offsets.reserve(count);

    if (count == 0)
        return offsets;

    int placed = 0;
    int ring = 0;

    while (placed < count) {
        float radius = ring * spacing;

        int slotsInRing = (ring == 0)
            ? 1
            : (int)floorf((2.0f * PI * radius) / spacing);

        slotsInRing = std::max(slotsInRing, 1);

        for (int i = 0; i < slotsInRing && placed < count; i++) {
            float angle = (2.0f * PI * i) / slotsInRing;

            offsets.push_back({
                cosf(angle) * radius,
                sinf(angle) * radius
            });

            placed++;
        }

        ring++;
    }

    return offsets;
}

void Infantry::rebuildFormation()
{
    formationOffsets = generateCircleFormation();

    float maxRadius = 0.0f;
    for (const auto& offset : formationOffsets) {
        maxRadius = std::max(maxRadius, Vector2Length(offset));
    }

    circle.radius = maxRadius + spacing * 0.5f;
}

Entity* Infantry::bestEnt(const std::vector<Entity*>& entities)
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
