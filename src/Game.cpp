#include "Game.h"

bool Game::make_play(int x, int y)
{
    bool color_to_move = play_count & 1;

    if (check_play(x, y))
    {
        b.set_point(x, y, color_to_move ? pointType::BLACK : pointType::WHITE);
        play_count++;
        return true;
    }
    return false;
}

bool Game::check_play(int x, int y)
{
    bool color_to_move = play_count & 1;

    struct nbrs neighbors = b.get_nbrs(x, y);

    return false;
}