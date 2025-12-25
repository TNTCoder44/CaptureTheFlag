#pragma once
#include <algorithm>
#include <vector>

#include "raylib.h"

enum class ColliderType {
    Circle,
    Rect
};

struct CircleCollider {
    float radius;
};

struct RectCollider {
    Vector2 halfSize;
};


class Entity
{
private:
    Texture2D texture;
    Vector2 position;
    int team; // 0 = player1 (server), 1 = player2 (client)
    float maxHealth;
    float health;
    int id;

    ColliderType colliderType;
    CircleCollider circle;
    RectCollider rect;

public:
    Entity(Vector2 pos, int team) : position(pos), team(team) {}
    virtual ~Entity() {}

    virtual int getID() const { return id; }
    virtual void setID(int newId) { id = newId; }

    virtual void setPosition(Vector2 pos) { position = pos; }
    virtual Vector2 getPosition() const { return position; } // get world position
    virtual int getTeam() const { return team; }
    virtual void setHealth(float hp) { health = hp;
	    health = std::max<float>(health, 0);
    }
	virtual float getHealth() const { return health; }
    virtual ColliderType getCollidorType() const { return colliderType; }
    virtual CircleCollider getCircleCollider() const { return circle; }
    virtual RectCollider getRectCollider() const { return rect; }

    virtual bool canAttack() const = 0;
    virtual float getAttackRange() const = 0;
	virtual float getDamage() const = 0;

	virtual void update(float dt, bool sf) {}   // update ability to attack based on if shots fired
    virtual void draw(bool inverted) {}

    virtual Entity* bestEnt(std::vector<Entity*> entities) = 0;
};
