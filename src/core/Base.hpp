#pragma once
#include "Entity.hpp"

class Base : public Entity
{
private:
    Texture2D textureNormal;
    Texture2D textureInverted;
    Vector2 position;

    int team;
    const float maxHealth = 1000.f;
  
    float health;
    int id;

    CircleCollider circle;

    Vector2 player1BasePos = {400, 975}; // 400, 750
    Vector2 player2BasePos = {400, -175}; // 400, 50

    Rectangle healthBarBounds; 

public:
    Base(Vector2 pos, int team);
    ~Base() override;

    int getID() const override { return id; }
    void setID(int newId) override { id = newId; }

    void setHealth(float hp) override
    {
        health = hp;
        health = std::max<float>(health, 0);
    }
    float getHealth() const override { return health; }
    void setPosition(Vector2 pos) override { }; // Base position is fixed
    void setDesiredPosition(Vector2 pos) override { };
    Vector2 getPosition() const override { return position; }
    int getTeam() const override { return team; }
    CircleCollider getCircleCollider() const override { return circle; }

    bool canAttack() const override { return false; };
    float getAttackRange() const override { return 0.0f; };
	float getDamage() const override { return 0; };
    bool getShooting() const override { return false; };

    void update(float dt, bool shotsFired) override;
    void draw(bool inverted) override;


    Entity* bestEnt(const std::vector<Entity*>& entities) override { return nullptr; };

private:
    Vector2 computeMovement(float dt) override { return {-1.f, -1.f}; };
};
