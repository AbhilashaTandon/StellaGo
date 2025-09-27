#include <iostream>
#include <random>
#include <cmath>

#include "Board.h"

Board::Board(int board_size) : boardsize(board_size)
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

    this->directions = {-boardsize - 2, -1, boardsize + 2, 1};

    zobrist = 0; // empty board state

    // set up random 64 bitstring generator
    std::random_device rd;

    std::mt19937_64 e2(rd());

    std::uniform_int_distribution<long long int> dist(std::llround(std::pow(2, 61)), std::llround(std::pow(2, 62)));

    std::cout << std::llround(std::pow(2, 61)) << std::endl;
    std::cout << std::llround(std::pow(2, 62)) << std::endl;

    for (int i = 0; i < boardsize * boardsize; i++)
    {
        zobrist_hashes_black.push_back(dist(e2));
        zobrist_hashes_white.push_back(dist(e2));
    }
}

pointType Board::get_point(int x, int y)
{
    return board[coords_to_idx(x, y)];
}

pointType Board::get_point(int idx)
{
    return board[idx];
}

void Board::set_point(int x, int y, pointType value)
{
    assert(value != pointType::BLANK);

    int idx = coords_to_idx(x, y);
    pointType current_state = board[idx];

    // xor to erase stone
    zobrist ^= (zobrist_hashes_black[x * boardsize + y]) * (current_state == pointType::BLACK && value == pointType::EMPTY);

    zobrist ^= (zobrist_hashes_white[x * boardsize + y]) * (current_state == pointType::WHITE && value == pointType::EMPTY);

    // xor to add stone
    zobrist ^= (zobrist_hashes_black[x * boardsize + y]) * (value == pointType::BLACK);
    zobrist ^= (zobrist_hashes_white[x * boardsize + y]) * (value == pointType::WHITE);

    board[idx] = value;
}

int Board::get_liberties(int x, int y)
{
    int num_liberties = 0;
    int idx = coords_to_idx(x, y);
    for (int i = 0; i < 4; i++)
    {
        if (board[idx + directions[i]] == pointType::EMPTY)
        {
            num_liberties++;
        }
    }
    return num_liberties;
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
                std::cout << "B ";
                break;
            case pointType::WHITE:
                std::cout << "W ";
                break;
            }
        }

        std::cout << std::endl;
    }
}

nbrs Board::get_nbrs(int x, int y)
{
    nbrs n;
    n.edges = 0;
    n.liberties = 0;
    n.black = 0;
    n.white = 0;
    uint8_t num_edges = 0;
    uint8_t num_libs = 0;
    uint8_t num_black = 0;
    uint8_t num_white = 0;

    int idx = coords_to_idx(x, y);

    for (uint8_t i = 0; i < 4; i++)
    {
        switch (board[idx + directions[i]])
        {
        case pointType::BLANK:
            num_edges++;
            n.edges |= uint8_t(1 << i);
            break;
        case pointType::EMPTY:
            num_libs++;
            n.liberties |= uint8_t(1 << i);
            break;
        case pointType::BLACK:
            num_black++;
            n.black |= uint8_t(1 << i);
            break;
        case pointType::WHITE:
            num_white++;
            n.white |= uint8_t(1 << i);
            break;
        }
    }

    assert((num_edges + num_libs + num_black + num_white) == 4);

    n.edges |= uint8_t(num_edges << 4);
    n.liberties |= uint8_t(num_libs << 4);
    n.black |= uint8_t(num_black << 4);
    n.white |= uint8_t(num_white << 4);
    return n;
}

int Board::coords_to_idx(int x, int y)
{
    assert(x >= 0 && x < boardsize);
    assert(y >= 0 && y < boardsize);
    return (boardsize + 2) * (y + 1) + x + 1;
}

std::pair<int, int> Board::idx_to_coords(int idx)
{
    assert(idx >= 0 && (unsigned int)idx < board.size());
    return std::pair<int, int>(idx / (boardsize + 2) - 1, idx % (boardsize + 2) - 1);
}
