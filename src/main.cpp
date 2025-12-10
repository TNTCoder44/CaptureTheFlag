#include "Game.hpp"

auto main() -> int 
{
    Game* game = new Game();
    game->run();
    delete game;
    return 0;
} 