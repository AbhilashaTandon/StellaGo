
#include <iostream>
#include <random>
#include <cmath>
#include <queue>

#include "Board.h"

void Board::check_position(uint16_t idx) const
{
    assert(chain_roots[idx] != 0);
    assert(chain_liberty_counts[chain_roots[idx]] != 0);
    assert(chain_sizes[chain_roots[idx]] != 0);
}

int16_t Board::score() const
{
    int black_liberties = 0;
    int white_liberties = 0;

    int black_eyes = 0;
    int white_eyes = 0;

    for (int i = 0; i < NUM_POINTS; i++)
    {
        int libs = chain_liberty_counts[i];
        switch (is_eye(i))
        {
        case BLACK:
            black_eyes++;
            break;
        case WHITE:
            white_eyes++;
            break;
        }

        if (libs == 0)
        {
            continue;
        }
        if (board[i] == pointType::BLACK)
        {
            black_liberties += chain_liberty_counts[i];
            if (libs < 3)
            {
                black_liberties -= chain_sizes[i];
            }
        }
        else
        {
            assert(board[i] == pointType::WHITE);
            white_liberties += chain_liberty_counts[i];
            if (libs < 3)
            {
                white_liberties -= chain_sizes[i];
            }
        }
    }

    int black_chain_score = 0;
    int white_chain_score = 0;

    for (int i = 0; i < NUM_POINTS; i++)
    {
        uint64_t chain_score = chain_sizes[i] * chain_sizes[i];
        if (chain_score != 0)
        {
            if (board[i] == BLACK)
            {
                black_chain_score += chain_score;
            }
            else if (board[i] == WHITE)
            {
                white_chain_score += chain_score;
            }
            else
            {
                assert(false);
            }
        }
    }

    return -komi + black_liberties - white_liberties + (black_chain_score >> 3) - (white_chain_score >> 3) + black_eyes * 3 - white_eyes * 3;
}

int Board::floodfill(pointType side) const
{
    uint16_t area_count = 0;
    std::array<bool, NUM_POINTS> visited{};
    std::queue<int> stones_to_check{};

    for (int i = 0; i < NUM_POINTS; i++)
    {
        if (board[i] != side)
        {
            continue;
        }

        stones_to_check.push(i);
        visited[i] = true;
        area_count++;
    }

    while (!stones_to_check.empty())
    {
        int starting_point = stones_to_check.front();
        stones_to_check.pop();
        for (int d = 0; d < 4; d++)
        {
            int neighbor = starting_point + directions[d];
            if (!visited[neighbor])
            {
                visited[neighbor] = true;
                stones_to_check.push(neighbor);
                area_count++;
            }
        }
    }

    return area_count;
}

float Board::area_score() const
{
    return ((float)floodfill(BLACK) - (float)floodfill(WHITE) - komi);
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
                    chain_liberty_locations[chain_id][neighbor] = true;
                    chain_liberty_counts[chain_id]++;
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

uint16_t Board::get_liberties(uint16_t idx) const
{
    uint16_t num_liberties = 0;
    for (uint16_t i = 0; i < 4; i++)
    {
        if (board[idx + directions[i]] == pointType::EMPTY)
        {
            num_liberties++;
        }
    }
    return num_liberties;
}