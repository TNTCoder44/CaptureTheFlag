#include "Infantrie.hpp"

Infantrie::Infantrie(Vector2 pos, int team) : Entity(pos, team)
{
    position = pos;
    this->team = team;

    if (team)
        texture = LoadTexture("../res/infantrie_1.png");
    else
        texture = LoadTexture("../res/infantrie_0.png");
}

Infantrie::~Infantrie()
{
    UnloadTexture(texture);
}
