#pragma once
#include "Entity.hpp"

class Infantrie : public Entity
{
private:
    Texture2D texture;
    Vector2 position;

    int team;

public:
    Infantrie(Vector2 pos, int team);
    ~Infantrie() override;

};
