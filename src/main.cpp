#include "Board.h"
#include <cstdio>
#include <random>
#include <ctime>
#include <iostream>
#include "Agent.h"
#include "SGFFile.h"

// int main()
// {
//     search_files();
//     return 0;
// }

// void stress_test()
// {
//     for (int i = 0; i < 10000; i++)
//     {
//         Board b = Board();
//         for (int j = 0; j < 1000; j++)
//         {
//             b.make_play(rand() % BOARD_SIZE, rand() % BOARD_SIZE);
//         }
//     }
// }

int main()
{
    //     srand(time(NULL));

    //     // std::vector<std::pair<int, int>> moves = {std::pair<int, int>(0, 1), std::pair<int, int>(3, 1), std::pair<int, int>(1, 2), std::pair<int, int>(2, 2), std::pair<int, int>(1, 0), std::pair<int, int>(2, 0), std::pair<int, int>(2, 1), std::pair<int, int>(1, 1), std::pair<int, int>(2, 1), std::pair<int, int>(1, 1), std::pair<int, int>(2, 1), std::pair<int, int>(1, 1)};

    // for (std::pair<int, int> coord : moves)
    // {
    //     if (g.make_play(coord.first, coord.second))
    //     {
    //         g.print_board();

    //     }
    // }
    //     //     std::cout << "Hello anyone there?" << std::endl;
    //     // #if DEBUG
    //     //     Board g = Board();
    //     //     for (int i = 0; i < 10000; i++)
    //     //     {
    //     //         g.make_play(rand() % BOARD_SIZE, rand() % BOARD_SIZE);
    //     //     }
    //     // #endif
    //     // #if PROFILE
    //     //     stress_test();
    //     // #endif

    //     //     std::cout << "Hello anyone there?" << std::endl;
    //     //     return 0;
    Agent a = Agent();
    a.play(3, 10);
}