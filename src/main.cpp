#include "Board.h"
#include "Game.h"

int main()
{
    Game g = Game(19);

    std::vector<std::pair<int, int>> moves = {std::pair(3, 3), std::pair(3, 3)};

    for (std::pair<int, int> coord : moves)
    {
        g.make_play(coord.first, coord.second);
        g.print_board();
    }

    return 0;
}