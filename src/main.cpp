#include "Board.h"
#include "Game.h"
#include <cstdio>
#include <random>

#define BOARDSIZE 19

int main()
{
    srand(2);

    Game g = Game(BOARDSIZE);

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