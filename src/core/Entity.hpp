#pragma once
#include "raylib.h"

class Entity
{
private:
    Texture2D texture;
    Vector2 position;
    int team; // 0 = red, 1 = blue

public:
    Entity(Vector2 pos, int team) : position(pos), team(team) {}
    virtual ~Entity() {}

    virtual void setPosition(Vector2 pos) { position = pos; }
    virtual Vector2 getPosition() const { return position; }
    virtual int getTeam() const { return team; }
    virtual void Draw() {}
};
