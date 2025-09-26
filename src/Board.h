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

#define NORTH 1        // 00000001
#define EAST (1 << 1)  // 00000010
#define SOUTH (1 << 2) // 00000100
#define WEST (1 << 3)  // 00001000
#define COUNT (7 << 4) // 01110000

struct nbrs
{
    uint8_t edges;
    uint8_t liberties;
    uint8_t black;
    uint8_t white;
};

class Board
{

public:
    Board(int boardsize);
    pointType get_point(int x, int y);
    pointType get_point(int idx);
    void set_point(int x, int y, pointType value);
    void set_point(int idx, pointType value);
    int get_liberties(int x, int y);
    // bool is_starpoint(int x, int y);
    void print_board();
    nbrs get_nbrs(int x, int y);
    int coords_to_idx(int x, int y);
    std::vector<int> directions;
    std::pair<int, int> idx_to_coords(int idx);

private:
    std::vector<pointType> board;
    int boardsize;
};

#endif