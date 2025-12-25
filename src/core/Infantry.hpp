#pragma once
#include "Entity.hpp"

class Infantry : public Entity
{
private:
    Texture2D texture;
    Vector2 position;

    int team;
    float cooldownTimer;

    const float attackCooldown = 2.f; // seconds
    const float maxHealth = 100.f;
    const float attackRange = 40.0f;
    const float damage = 10.0f;     // 10 dmg / s

    float health;
    int id;

    ColliderType colliderType;
    CircleCollider circle;
    RectCollider rect;

public:
    Infantry(Vector2 pos, int team);
    ~Infantry() override;

    int getID() const override { return id; }
    void setID(int newId) override { id = newId; }

    bool canAttack() const override;
    float getAttackRange() const override { return attackRange; }
    float getDamage() const override { return damage; }
	void setHealth(float hp) override {
        health = hp;
        health = std::max<float>(health, 0);
    }
	float getHealth() const override { return health; }
	void setPosition(Vector2 pos) override{ position = pos; }
	Vector2 getPosition() const override { return position; }
    int getTeam() const override { return team; }
    ColliderType getCollidorType() const override { return colliderType; }
    CircleCollider getCircleCollider() const override { return circle; }
    RectCollider getRectCollider() const override { return rect; }

    void update(float dt, bool shotsFired) override;
    void draw(bool inverted) override;

	Entity* bestEnt(std::vector<Entity*> entities) override;
};
