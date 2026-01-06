#pragma once
#include "Entity.hpp"

class Artillery : public Entity
{
private:
    Texture2D textureFull;
    Texture2D textureShooting;
    Vector2 position;

    int team;
    float cooldownTimer;

    const float attackCooldown = 2.f; // seconds
    const float maxHealth = 200.f;
    float attackRange = 100.0f;
    const float damage = 20.0f; // 10 dmg / s

    float speed = 10.f; // units per second

    float health;
    int id;

    Vector2 desiredPosition;

    CircleCollider circle;

    bool isShooting;

    Rectangle healthBarBounds; 

public:
    Artillery(Vector2 pos, int team, Vector2 desiredPos = {-1.f, -1.f});
    ~Artillery() override;

    int getID() const override { return id; }
    void setID(int newId) override { id = newId; }

    bool canAttack() const override;
    float getAttackRange() const override { return attackRange; }
    float getDamage() const override { return damage; }
    void setHealth(float hp) override
    {
        health = hp;
        health = std::max<float>(health, 0);
    }
    float getHealth() const override { return health; }
    void setPosition(Vector2 pos) override { position = pos; }
    void setDesiredPosition(Vector2 pos) override { desiredPosition = pos; }
    Vector2 getPosition() const override { return position; }
    int getTeam() const override { return team; }
    CircleCollider getCircleCollider() const override { return circle; }
    bool getShooting() const override { return isShooting; }

    void update(float dt, bool shotsFired) override;
    void draw(bool inverted) override;

    Entity *bestEnt(const std::vector<Entity *> &entities) override;

private:
    Vector2 computeMovement(float dt) override;
};
