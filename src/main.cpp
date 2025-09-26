#include "Board.h"
#include "Game.h"
#include <cstdio>
#include <random>

#define BOARDSIZE 19

int main()
{
    srand(2);

    Game g = Game(BOARDSIZE);

    // std::vector<std::pair<int, int>> moves = {std::pair(3, 3), std::pair(3, 3), std::pair(3, 1), std::pair(3, 2), std::pair(1, 2), std::pair(3, 1), std::pair(0, 3)};

    // for (std::pair<int, int> coord : moves)
    // {
    //     if (g.make_play(coord.first, coord.second))
    //     {
    //         g.print_board();
    //     }
    // }

    for (int i = 0; i < 100000; i++)
    {
        if (g.make_play(rand() % BOARDSIZE, rand() % BOARDSIZE))
        {
            g.print_board();
        }
        g.check_for_errors();
    }

    return 0;
}