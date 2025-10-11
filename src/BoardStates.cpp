
#include <iostream>
#include <random>
#include <cmath>

#include "Board.h"

uint16_t Board::coords_to_idx(uint16_t x, uint16_t y) const
{
#if DEBUG
    assert(x >= 0 && x < BOARD_SIZE);
    assert(y >= 0 && y < BOARD_SIZE);
#endif
    return (BOARD_SIZE + 2) * (y + 1) + x + 1;
}
std::pair<int, int> Board::idx_to_coords(uint16_t idx) const
{
#if DEBUG
    assert(idx >= 0 && (unsigned int)idx < board.size());
#endif
    return std::pair<int, int>(idx / (BOARD_SIZE + 2) - 1, idx % (BOARD_SIZE + 2) - 1);
}

void Board::check_position(uint16_t idx) const
{
    assert(chain_roots[idx] != 0);
    assert(chain_liberties[chain_roots[idx]] != 0);
    assert(chain_sizes[chain_roots[idx]] != 0);
}

int16_t Board::score() const
{
    int black_liberties = 0;
    int white_liberties = 0;
    for (int i = 0; i < NUM_POINTS; i++)
    {
        if (chain_liberties[i] == 0)
        {
            continue;
        }
        if (board[i] == pointType::BLACK)
        {
            black_liberties += chain_liberties[i];
        }
        else
        {
            assert(board[i] == pointType::WHITE);
            white_liberties += chain_liberties[i];
        }
    }
    return int16_t(black_count) - int16_t(white_count) - komi + black_liberties - white_liberties;
}

pointType Board::get_point(uint16_t idx) const
{
    return board[idx];
}

void Board::set_point(uint16_t idx, pointType value)
{
    pointType current_state = board[idx];
#if DEBUG
    assert(value != pointType::BLANK);
    assert(current_state != value);
    assert(!(current_state == BLACK && value == WHITE));
    assert(!(current_state == WHITE && value == BLACK));
#endif

    // xor to erase stone
    zobrist ^= (zobrist_hashes_black[idx]) * (current_state == pointType::BLACK && value == pointType::EMPTY);

    zobrist ^= (zobrist_hashes_white[idx]) * (current_state == pointType::WHITE && value == pointType::EMPTY);

    // xor to add stone
    zobrist ^= (zobrist_hashes_black[idx]) * (value == pointType::BLACK);
    zobrist ^= (zobrist_hashes_white[idx]) * (value == pointType::WHITE);

    board[idx] = value;

    switch (current_state)
    {
    case pointType::EMPTY:
        black_count += (value == pointType::BLACK);
        white_count += (value == pointType::WHITE);
        empty_count--;
        break;
    case pointType::BLACK:
        black_count--;
        empty_count++;
        break;
    case pointType::WHITE:
        white_count--;
        empty_count++;
        break;
    case BLANK:
        assert(false);
        break;

        //     default:
        // #if DEBUG
        //         assert(false);
        // #endif
    }

    if (value == pointType::EMPTY)
    {
        std::array<int, 4> prev_chains{};
        uint16_t prev_chain_count = 0;
        for (uint16_t i = 0; i < 4; i++)
        {
            uint16_t neighbor = idx + directions[i];
            if (chain_roots[neighbor] != 0)
            {
                uint16_t chain_id = chain_roots[neighbor];

                bool dupl = false;
                for (uint16_t x = 0; x < prev_chain_count; x++)

                {
                    if (prev_chains[x] == chain_id)
                    {
                        dupl = true;
                        break;
                    }
                }

                if (!dupl)
                {
                    prev_chains[prev_chain_count] = chain_id;
                    prev_chain_count++;
                    chain_liberties[chain_id]++;
                }
            }
        }
    }
}

nbrs Board::get_nbrs(uint16_t idx) const
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

#if DEBUG
    assert((num_edges + num_libs + num_black + num_white) == 4);
#endif

    n.edges |= uint8_t(num_edges << 4);
    n.liberties |= uint8_t(num_libs << 4);
    n.black |= uint8_t(num_black << 4);
    n.white |= uint8_t(num_white << 4);
    return n;
}