#include <iostream>
#include "Board.h"

Board::Board(int board_size)
{
    this->boardsize = board_size;
    this->board = std::vector<pointType>((board_size + 2) * (board_size + 2), pointType::EMPTY);

    for (int i = 0; i < board_size + 1; i++)
    {
        // add in edges of board
        board[i + 1] = pointType::BLANK;
        board[(i) * (board_size + 2)] = pointType::BLANK;
        board[(board_size + 1) * (board_size + 2) + i] = pointType::BLANK;
        board[(i + 1) * (board_size + 2) + (board_size + 1)] = pointType::BLANK;
    }
}

pointType Board::get_point(int x, int y)
{
    return board[coords_to_idx(x, y)];
}

void Board::set_point(int x, int y, pointType value)
{
}

int Board::get_liberties(int x, int y)
{
    return 0;
}

bool Board::is_starpoint(int x, int y)
{
    return false;
}

void Board::print_board()
{
    for (int i = 0; i < (boardsize + 2); i++)
    {
        for (int j = 0; j < (boardsize + 2); j++)
        {
            switch (board[i * (boardsize + 2) + j])
            {
            case pointType::BLANK:
                std::cout << "# ";
                break;
            case pointType::EMPTY:
                std::cout << "  ";
                break;
            case pointType::BLACK:
                std::cout << "O ";
                break;
            case pointType::WHITE:
                std::cout << "X ";
                break;
            }
        }

        std::cout << std::endl;
    }
}

nbrs Board::get_nbrs(int x, int y)
{
    return nbrs();
}

int Board::coords_to_idx(int x, int y)
{
    return (boardsize + 2) * (y + 1) + x + 1;
}

std::pair<int, int> Board::idx_to_coords(int idx)
{
    return std::pair<int, int>(idx / (boardsize + 2) - 1, idx % (boardsize + 2) - 1);
}
