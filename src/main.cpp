#include "Board.h"
#include "Game.h"
#include <cstdio>
#include <random>

#define BOARDSIZE 19

int main()
{
    srand(3);

    Game g = Game(BOARDSIZE);

    // std::vector<std::pair<int, int>> moves = {std::pair<int, int>(0, 1), std::pair<int, int>(3, 1), std::pair<int, int>(1, 2), std::pair<int, int>(2, 2), std::pair<int, int>(1, 0), std::pair<int, int>(2, 0), std::pair<int, int>(2, 1), std::pair<int, int>(1, 1), std::pair<int, int>(2, 1), std::pair<int, int>(1, 1), std::pair<int, int>(2, 1), std::pair<int, int>(1, 1)};

    // for (std::pair<int, int> coord : moves)
    // {
    //     if (g.make_play(coord.first, coord.second))
    //     {
    //         g.print_board();
    //     }
    // }

    for (int x = 0; x < 1000; x++)
    {
        for (int i = 0; i < 10000; i++)
        {
            if (g.make_play(rand() % BOARDSIZE, rand() % BOARDSIZE))
            {
                g.print_board();
            }
            // g.check_for_errors();
        }
    }

    return 0;
}