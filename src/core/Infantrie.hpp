#pragma once
#include "Entity.hpp"

class Infantrie : public Entity
{
private:
    Texture2D texture;
    Vector2 position;

    int team;
    float cooldownTimer = 0.f;

    const float attackCooldown = 1.f; // seconds
    const float maxHealth = 100.f;
    const float attackRange = 40.0f;
    const float damage = 10.0f;

public:
    float health;
    int id;

public:
    Infantrie(Vector2 pos, int team);
    ~Infantrie() override;

    bool CanAttack() const override;
    float GetAttackRange() const override { return attackRange; }
    float GetDamage() const override { return damage; }

    void Update(float dt) override;
    void Draw(bool inverted) override;


};
