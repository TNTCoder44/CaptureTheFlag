#pragma once
#include "Entity.hpp"

class Infantry : public Entity
{
private:
    Texture2D textureFull;
    Texture2D textureInjured;
    Texture2D textureInjured2;
    Vector2 position;

    int team;
    float cooldownTimer;

    const float attackCooldown = 1.f; // seconds
    const float maxHealth = 100.f;
	float attackRange = 40.0f;
    const float damage = 10.0f;     // 10 dmg / s
    const int soldierSize = 100;    
    const int maxSoldiers = 7;   // maximum number of soldiers in the infantry unit
	
    float speed = 20.f;        // units per second
    float spacing;

    std::vector<Vector2> formationOffsets;
    int soldiersAlive;

    float health;
    int id;

    Vector2 desiredPosition;

    CircleCollider circle;

	bool isShooting;

public:
    Infantry(Vector2 pos, int team, Vector2 desiredPos = { -1.f,-1.f });
    ~Infantry() override;

    int getID() const override { return id; }
    void setID(int newId) override { id = newId; }

    bool canAttack() const override;
    float getAttackRange() const override { return attackRange; }
    float getDamage() const override { return damage; }
	void setHealth(float hp) override {
        health = hp;
        health = std::max<float>(health, 0);

        soldiersAlive = static_cast<int>(ceil(float(health / (maxHealth / maxSoldiers))));
        rebuildFormation();
    }
	float getHealth() const override { return health; }
	void setPosition(Vector2 pos) override{ position = pos; }
	Vector2 getPosition() const override { return position; }
    int getTeam() const override { return team; }
    CircleCollider getCircleCollider() const override { return circle; }
	bool getShooting() const override { return isShooting; }

    void update(float dt, bool shotsFired) override;
    void draw(bool inverted) override;
    

	Entity* bestEnt(const std::vector<Entity*>& entities) override;

private:
    Vector2 computeMovement(float dt) override;
    std::vector<Vector2> generateCircleFormation();
    void rebuildFormation();
};
