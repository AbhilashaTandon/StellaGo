#include "Board.h"
#include <cstdio>
#include <random>
#include <ctime>
#include <iostream>
#include <string>
#include "Agent.h"
#include "SGFFile.h"

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        std::cerr << "Error: program must be started by specifying the board size and side to play. Format your command like so\n\nstella.exe (board size) (b or w for black or white) (optional flags)\n\n";
        return -1;
    }
    std::string board_size_arg = argv[1];
    int board_size = 19;
    try
    {
        board_size = std::stoi(board_size_arg);
    }
    catch (const std::invalid_argument &e)
    {
        std::cerr << "Error: " << board_size_arg << " is not a valid board size\n\n";
        return -1;
    }
    if (board_size < 5)
    {
        std::cerr << "Error: Board size must be at least 5.\n\n";

        return -1;
    }
    if (board_size > 19)
    {
        std::cerr << "Error: Board size must be at most 19.\n\n";
        return -1;
    }
    if (board_size % 2 == 0)
    {
        std::cerr << "Error: Board size must be an odd number.\n\n";
        return -1;
    }

    std::string side_arg = argv[2];
    bool side_to_play = true;
    if (side_arg == "w")
    {
        side_to_play = false;
    }
    else if (side_arg != "b")
    {
        std::cerr << "Error: second argument must be either 'b' or 'w', specifying which side the bot is to play.\n\n";
        return -1;
    }

    std::cout << "Board size " << board_size << " side " << side_to_play << "\n\n";
}
// {
//     Agent a = Agent();
//     a.play(3, 100);
// }