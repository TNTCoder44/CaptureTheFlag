#include "Button.hpp"

#include <iostream>

#define NUM_FRAMES 3

Button::Button(const char *imagePath, Vector2 imagePosition, float scale)
{
    init(imagePath, imagePosition, scale);
}

Button::Button(Button &&other) noexcept
{
    *this = std::move(other);
}

Button &Button::operator=(Button &&other) noexcept
{
    if (this == &other)
        return *this;

    if (initialized && texture.id != 0)
        UnloadTexture(texture);

    texture = other.texture;
    position = other.position;
    scale = other.scale;
    frameHeight = other.frameHeight;
    sourceRec = other.sourceRec;
    btnBounds = other.btnBounds;
    btnState = other.btnState;
    btnAction = other.btnAction;
    initialized = other.initialized;

    other.texture = Texture2D{};
    other.initialized = false;
    return *this;
}

Button::~Button()
{
    if (initialized && texture.id != 0)
        UnloadTexture(texture);
}

void Button::init(const char *imagePath, Vector2 imagePosition, float scale)
{
    if (initialized && texture.id != 0)
    {
        UnloadTexture(texture);
        texture = Texture2D{};
        initialized = false;
    }

    Image image = LoadImage(imagePath);
    if (image.data == nullptr || image.width <= 0 || image.height <= 0)
    {
        printf("Button::init: LoadImage failed for '%s'\n", imagePath);
        if (image.data != nullptr)
            UnloadImage(image);
        return;
    }

    int originalWidth = image.width;
    int originalHeight = image.height;

    int newWidth = static_cast<int>(originalWidth * scale);
    int newHeight = static_cast<int>(originalHeight * scale);

    ImageResize(&image, newWidth, newHeight);
    texture = LoadTextureFromImage(image);
    UnloadImage(image);

    position = imagePosition;
    this->scale = scale;

    frameHeight = (float)texture.height / NUM_FRAMES;
    sourceRec = {0, 0, (float)texture.width, frameHeight};
    btnBounds = {imagePosition.x, imagePosition.y, (float)texture.width, frameHeight};

    initialized = (texture.id != 0);
    if (!initialized)
        printf("Button::init: LoadTextureFromImage failed for '%s'\n", imagePath);
}

void Button::draw()
{
    if (!initialized)
        return;
    sourceRec.y = btnState * frameHeight;
    DrawTextureRec(texture, sourceRec, Vector2{btnBounds.x, btnBounds.y}, WHITE); // Draw button frame
}

void Button::update(Vector2 mousePos)
{
    if (!initialized)
        return;
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