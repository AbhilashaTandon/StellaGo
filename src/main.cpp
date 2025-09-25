#include "Board.h"
#include "Game.h"

int main(int argc, char *argv[])
{
    Board b = Board(19);
    b.print_board();
    return 0;
}