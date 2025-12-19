#pragma once
#include <raylib.h>

class Button
{
    public:
        Button(const char* imagePath, Vector2 imagePosition, float scale);
        ~Button();
        void Draw();
        void Update(Vector2 mousePos);

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
        Texture2D texture;
        Vector2 position;
        float scale;

        // Define frame rectangle for drawing
        float frameHeight;
        Rectangle sourceRec;

        // Define button bounds on screen
        Rectangle btnBounds;

        int btnState = 0;               // Button state: 0-NORMAL, 1-MOUSE_HOVER, 2-PRESSED
        bool btnAction = false;         // Button action should be activated through clicking
};