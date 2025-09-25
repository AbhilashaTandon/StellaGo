#ifndef BOARD_H
#define BOARD_H

#include <cstdint>
#include <vector>
#include <cassert>

enum pointType
{
    BLANK = 0,
    EMPTY = 1,
    BLACK = 2,
    WHITE = 3
};

struct nbrs
{
    int num_edges;
    int num_liberties;
    int num_black_nbrs;
    int num_white_nbrs;
    uint8_t edges;
    uint8_t liberties;
    uint8_t black_nbrs; // NESW, bit is a 1 if that direction has a neighbor of the given type
    uint8_t white_nbrs;
};

class Board
{

public:
    Board(int board_size);
    pointType get_point(int x, int y);
    void set_point(int x, int y, pointType value);
    int get_liberties(int x, int y);
    bool is_starpoint(int x, int y);
    void print_board();
    nbrs get_nbrs(int x, int y);
    const int directions[4] = {-boardsize - 2, -1, boardsize + 2, 1};

private:
    int coords_to_idx(int x, int y);
    std::pair<int, int> idx_to_coords(int idx);
    std::vector<pointType> board;
    int boardsize;
};

#endif