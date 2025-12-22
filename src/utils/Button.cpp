#include "Button.hpp"

#define NUM_FRAMES 3

Button::Button(const char *imagePath, Vector2 imagePosition, float scale)
{
    Image image = LoadImage(imagePath);

    int originalWidth = image.width;
    int originalHeight = image.height;

    int newWidth = static_cast<int>(originalWidth * scale);
    int newHeight = static_cast<int>(originalHeight * scale);

    ImageResize(&image, newWidth, newHeight);
    texture = LoadTextureFromImage(image);
    UnloadImage(image);
    position = imagePosition;

    frameHeight = (float)texture.height / NUM_FRAMES;
    sourceRec = {0, 0, (float)texture.width, frameHeight};
    btnBounds = {imagePosition.x, imagePosition.y, (float)texture.width, frameHeight};
}

Button::~Button()
{
    UnloadTexture(texture);
}

void Button::Draw()
{
    sourceRec.y = btnState * frameHeight;
    DrawTextureRec(texture, sourceRec, Vector2{btnBounds.x, btnBounds.y}, WHITE); // Draw button frame
}

void Button::Update(Vector2 mousePos)
{
    Rectangle rect = {position.x, position.y, static_cast<float>(texture.width), static_cast<float>(frameHeight)};

    if (CheckCollisionPointRec(mousePos, rect))
    {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
            btnState = 2;
        else
            btnState = 1;

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
            btnAction = true;
    }
    else
        btnState = 0;
}