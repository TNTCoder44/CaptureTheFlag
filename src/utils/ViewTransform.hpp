#pragma once
#include <raylib.h>

inline Vector2 WorldToView(Vector2 worldPos, bool inverted)
{
    if(!inverted)
        return worldPos;

    float worldHeight = GetScreenHeight(); // 800.f 
    float worldWidth = GetScreenWidth(); // 800.f

    return { worldWidth - worldPos.x, worldHeight - worldPos.y }; // invert y/x axis for player 
}

// for revieving input 
inline Vector2 ViewToWorld(Vector2 screenPos, bool inverted)
{
    if(!inverted)
        return screenPos;

    float worldHeight = GetScreenHeight(); // 800.f 
    float worldWidth = GetScreenWidth(); // 800.f

    return { worldWidth - screenPos.x, worldHeight - screenPos.y }; // invert y/x axis for player 
}


// for inverse rendering 
inline void DrawEntityTexture(
    Texture2D tex,
    Vector2 pos,
    Vector2 size,
    bool inverted
) {
    Rectangle src = {
        0, 0,
        (float)tex.width,
        (float)tex.height
    };

    // flip the texture if needed
    if (inverted) {
        src.height *= -1;
        src.width *= -1;
    }

    Rectangle dst = {
        pos.x,
        pos.y,
        size.x,
        size.y
    };

    Vector2 origin = { size.x / 2, size.y / 2 };

    DrawTexturePro(tex, src, dst, origin, 0.0f, WHITE);
}
