#pragma once
#include "raylib.h"

class Entity
{
private:
    Texture2D texture;
    Vector2 position;
    int team; // 0 = player1 (server), 1 = player2 (client)
    float maxHealth;

public:
    float health;
    int id;

public:
    Entity(Vector2 pos, int team) : position(pos), team(team) {}
    virtual ~Entity() {}

    virtual void setPosition(Vector2 pos) { position = pos; }
    virtual Vector2 getPosition() const { return position; } // get world position
    virtual int getTeam() const { return team; }

    virtual bool CanAttack() const { return false; }
    virtual float GetAttackRange() const { return 0.0f; }
    virtual float GetDamage() const { return 0.0f; }

    virtual void Update(float dt) {}
    virtual void Draw(bool inverted) {}
};
