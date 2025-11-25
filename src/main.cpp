#include "Board.h"
#include <cstdio>
#include <random>
#include <ctime>
#include <iostream>
#include <string>
#include "Bot.h"
#include "SGFReader.h"

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        std::cerr << "Error: program must be started by specifying the side to play and search depth. Format your command like so\n\nstella.exe (b or w for black or white) (search depth) (optional flags)\n\n";
        return -1;
    }

    std::string side_arg = argv[1];
    bool side_to_play = true;
    if (side_arg == "w")
    {
        side_to_play = false;
    }
    else if (side_arg != "b")
    {
        std::cerr << "Error: first argument must be either 'b' or 'w', specifying which side the bot is to play.\n\n";
        return -1;
    }

    std::string depth_arg = argv[2];
    int depth = 1;
    try
    {
        depth = std::stoi(depth_arg);
    }
    catch (const std::invalid_argument &e)
    {
        std::cerr << "Error: " << depth_arg << " is not a valid depth\n\n";
        return -1;
    }
    if (depth < 1)
    {
        std::cerr << "Error: Depth must be at least 1.\n\n";

        return -1;
    }

    std::cout << "Side " << side_to_play << " depth " << depth << "\n\n";

    Bot bot = Bot();

    int play_count = 0;

    while (bot.get_game_result() == EMPTY)
    {
        if (side_to_play == (play_count % 2 == 0))
        {
            int move = bot.make_best_play(depth);
            // print move
        }
        else
        {
            // read move from stdin
            // if stdin is "quit" then exit
            // if stdin is "pass" then pass
            // if stdin is "resign" then resign
        }
        // print board
    }

    // game loop
}
// {
//     Bot a = Bot();
//     a.play(3, 100);
// }