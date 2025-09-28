#ifndef BOARD_H
#define BOARD_H
#include "Config.h"

#include <cstdint>
// #include <vector>
#include <array>
#include <cassert>

enum pointType
{
    BLANK = 0,
    EMPTY = 1,
    BLACK = 2,
    WHITE = 3
};

#define NORTH 1        // 00000001
#define WEST (1 << 1)  // 00000010
#define SOUTH (1 << 2) // 00000100
#define EAST (1 << 3)  // 00001000
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
    Board();
    Board(const Board &b);

    bool make_play(uint16_t x, uint16_t y);
    bool whose_turn();
    uint16_t get_play_count();
    void print_board();
    void check_for_errors();

protected:
    std::array<pointType, NUM_POINTS> board{};

    uint64_t zobrist;
    std::array<uint64_t, NUM_POINTS> zobrist_hashes_black{};
    std::array<uint64_t, NUM_POINTS> zobrist_hashes_white{};

    std::array<uint16_t, NUM_POINTS> chain_roots{};
    // start of chain list
    std::array<uint16_t, NUM_POINTS> chain_liberties{}; // only defined for locations that are roots
    std::array<uint16_t, NUM_POINTS> chain_sizes{};     // also only defined for roots, num stones for chain

    uint16_t white_count;
    uint16_t black_count;
    uint16_t empty_count;

    void set_point(uint16_t idx, pointType value);

    bool check_play(uint16_t idx);
    bool is_suicide(uint16_t idx);
    void update_chains(uint16_t idx);

    void create_chain(uint16_t idx);
    void extend_chain(uint16_t idx, uint16_t adj_stone);
    void merge_chains(std::array<uint16_t, 4> chain_neighbors, uint16_t num_chains, uint16_t idx);
    void capture_chain(uint16_t chain_id);

    nbrs get_nbrs(uint16_t idx);
    uint64_t get_hash();
    pointType get_point(uint16_t idx);
    uint16_t get_liberties(uint16_t idx);

    uint16_t coords_to_idx(uint16_t x, uint16_t y);
    std::array<int, 4> directions;
    std::pair<int, int> idx_to_coords(uint16_t idx);

    uint16_t play_count;
    uint64_t black_ko_hash;
    uint64_t white_ko_hash;

    void check_position(uint16_t idx);
};

#endif