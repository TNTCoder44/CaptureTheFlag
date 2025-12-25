#include "Infantrie.hpp"
#include "../utils/Filesystem.hpp"
#include "../utils/ViewTransform.hpp"

Infantrie::Infantrie(Vector2 pos, int team) : Entity(pos, team)
{
    position = pos;
    this->team = team;

    health = maxHealth;

    if (team == 0)
        texture = LoadTexture(FileSystem::getPath("res/player1.png").c_str());
    else if (team == 1)
        texture = LoadTexture(FileSystem::getPath("res/player2.png").c_str());
    else
        throw std::runtime_error("Invalid team for Infantrie entity");
    
}

Infantrie::~Infantrie()
{
    UnloadTexture(texture);
}

bool Infantrie::CanAttack() const
{
    if (cooldownTimer >= 0)
        return true;
    
}

void Infantrie::Update(float dt)
{
    cooldownTimer += dt;
}

void Infantrie::Draw(bool inverted)
{
    // if it is drawn from the client side, the texture will always be drawn inverted on the other side, no matter what team it belongs to
    if (inverted)
    {
        Vector2 viewPos = WorldToView(position, inverted); // from world to view coordinates
        DrawEntityTexture(texture, viewPos, { (float)texture.width, (float)texture.height }, team == 0); // flipped if other player
    }
    else
    {
        // server = world coordinates
        DrawEntityTexture(texture, position, { (float)texture.width, (float)texture.height }, team == 1); // flipped if other player
    }
    
    printf("I was here\n");
    
}
