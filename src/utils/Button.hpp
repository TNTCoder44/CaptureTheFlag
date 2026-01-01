#pragma once
#include <raylib.h>

class Button
{
public:
    Button() = default;
    Button(const char *imagePath, Vector2 imagePosition, float scale);
    ~Button();

    Button(const Button &) = delete;
    Button &operator=(const Button &) = delete;
    Button(Button &&other) noexcept;
    Button &operator=(Button &&other) noexcept;

    void init(const char *imagePath, Vector2 imagePosition, float scale);
    void draw();
    void update(Vector2 mousePos);

    inline bool isPressed()
    {
        if (btnAction)
        {
            btnAction = false;
            return true;
        }
        return false;
    }

private:
    Texture2D texture{};
    bool initialized = false;
    Vector2 position;
    float scale;

    // Define frame rectangle for drawing
    float frameHeight;
    Rectangle sourceRec;

    // Define button bounds on screen
    Rectangle btnBounds;

    int btnState = 0;       // Button state: 0-NORMAL, 1-MOUSE_HOVER, 2-PRESSED
    bool btnAction = false; // Button action should be activated through clicking
};