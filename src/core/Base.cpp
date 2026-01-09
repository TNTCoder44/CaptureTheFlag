#include "Base.hpp"

#include <iostream>
#include <stdexcept>

#include "../utils/Filesystem.hpp"
#include "../utils/ViewTransform.hpp"
#include "../utils/Math.hpp"

Base::Base(Vector2 pos, int team) : Entity(pos, team)
{
    position = pos;
    this->team = team;

    health = maxHealth;             // 1000.f

    if (team == 0)
    {
        textureNormal = LoadTexture(FileSystem::getPath("res/base/blue_base.png").c_str());
        textureInverted = LoadTexture(FileSystem::getPath("res/base/blue.png").c_str());
    }
    else if (team == 1)
    {
        textureNormal = LoadTexture(FileSystem::getPath("res/base/red_base.png").c_str());
        textureInverted = LoadTexture(FileSystem::getPath("res/base/red.png").c_str());
    }
    else
        throw std::runtime_error("Invalid team for Base entity");

    // set collider
    circle.radius = 300.f;
}

Base::~Base()
{
    UnloadTexture(textureNormal);
    UnloadTexture(textureInverted);
}

void Base::update(float dt, bool shotsFired)
{
    // Base does not move
}

void Base::draw(bool inverted)
{
    // calculate color
    float ratio = health / maxHealth;
    Color barColor = math::HealthToColor(ratio);

    Texture2D texture;

    if (inverted)
        texture = team == 0 ? textureInverted : textureNormal; 
    else
        texture = team == 0 ? textureNormal : textureInverted;

    position = team == 1 ? player2BasePos : player1BasePos;

    // store original position
    auto pos = position;

    if (inverted || team == 0)
    {
        if (inverted && team == 1)
            pos.y += 225.f;
        else
            pos.y -= 225.f;
    }
    else
    {
        pos.y += 225.f;
    }

    auto scale = 0.3f;

    Vector2 viewPos = WorldToView(pos, inverted);

    float offset;
    if (inverted)
        if(team == 0)
            offset = BAR_OFFSET_Y_UP;
        else
            offset = BAR_OFFSET_Y_DOWN;
    else
        if(team == 0)
            offset = BAR_OFFSET_Y_DOWN;
        else
            offset = BAR_OFFSET_Y_UP;
        
    Rectangle back = {
        viewPos.x - BAR_WIDTH / 2.0f,
        viewPos.y + offset,
        BAR_WIDTH,
        BAR_HEIGHT
    };

    Rectangle front = back;
    front.width *= ratio;

    // if it is drawn from the client side, the texture will always be drawn inverted on the other side, no matter what team it belongs to
    if (inverted)
    {                                    // from world to view coordinates
        DrawEntityTexture(texture, viewPos, {(float)texture.width, (float)texture.height}, team == 0, scale); // flipped if other player
    }
    else
    {
        // server = world coordinates
        DrawEntityTexture(texture, pos, {(float)texture.width, (float)texture.height}, team == 1, scale); // flipped if other player
    }

    // background
    DrawRectangleRec(back, DARKGRAY);

    // foreground - color
    DrawRectangleRec(front, barColor);

    // frame
    DrawRectangleLinesEx(back, 1.0f, BLACK);
}
