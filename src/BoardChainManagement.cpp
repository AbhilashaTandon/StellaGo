#include <iostream>
#include <random>
#include <cmath>

#include "Board.h"

void Board::create_chain(uint16_t idx)
{
    chain_roots[idx] = idx;
    chain_liberties[idx] = get_liberties(idx);
    chain_sizes[idx] = 1;
}

void Board::extend_chain(uint16_t idx, uint16_t adj_stone)
{
    uint16_t chain_id = chain_roots[adj_stone];
    chain_roots[idx] = chain_id;
    chain_sizes[chain_id]++;
    chain_liberties[chain_id]--;

    for (uint16_t i = 0; i < 4; i++)
    {
        if (board[idx + directions[i]] == pointType::EMPTY)
        // for each liberty of the new stone
        {
            bool already_counted = false;
            for (uint16_t j = 0; j < 4; j++)
            {
                if (directions[i] + directions[j] == 0)
                {
                    continue;
                }
                if (chain_roots[idx + directions[i] + directions[j]] == chain_id)
                {
                    // check that it's not already counted as a chain liberty
                    already_counted = true;
                    break;
                }
            }
            if (!already_counted)
            {
                chain_liberties[chain_id]++;
                // if a liberty of the new stone doesn't border the chain already then add it for a new liberty
            }
        }
    }
}

void Board::merge_chains(std::array<uint16_t, 4> neighbor_roots, uint16_t num_neighbors, uint16_t idx)
{
#if DEBUG
    assert(num_neighbors > 1);
    for (uint16_t i = 0; i < num_neighbors; i++)
    {
        if (i < num_neighbors)
        {
            assert(neighbor_roots[i] != 0);
        }
        else
        {
            assert(neighbor_roots[i] == 0);
        }
    }
#endif

    uint16_t new_root = neighbor_roots[0];
    // pick root of first chain to be root of new merged one

    uint16_t chain_size = chain_sizes[new_root] + 1;
    // add 1 for new stone

    for (uint16_t i = 1; i < num_neighbors; i++)
    {
        chain_size += chain_sizes[neighbor_roots[i]];
        chain_sizes[neighbor_roots[i]] = 0;
        // sum collective size and clear out afterwards
    }

    for (uint16_t i = 0; i < num_neighbors; i++)
    {
        chain_liberties[neighbor_roots[i]] = 0;
        // update sizes and liberties
    }
    chain_sizes[new_root] = chain_size;

    // update chain roots

    for (uint16_t i = 0; i < NUM_POINTS; i++)
    {
        if (chain_roots[i] == 0)
        {
            // if empty
            continue;
        }
        for (uint16_t j = 1; j < num_neighbors; j++)
        {

            if (chain_roots[i] == neighbor_roots[j])
            {
                chain_roots[i] = new_root;
                break;
            }
        }
    }

    chain_roots[idx] = new_root;

    uint16_t num_liberties = 0;
    for (uint16_t i = 0; i < NUM_POINTS; i++)
    {
        if (board[i] == pointType::EMPTY)
        {
            if (i == idx)
            {
                continue;
            }
            for (uint16_t j = 0; j < 4; j++)
            {
                if (chain_roots[i + directions[j]] == new_root)
                {
                    num_liberties++;
                    break;
                }
            }
        }
    }

    chain_liberties[new_root] = num_liberties;
}

void Board::capture_chain(uint16_t chain_root)
{
#if DEBUG
    assert(chain_liberties[chain_root] == 0);
#endif
    chain_sizes[chain_root] = 0;

    for (uint16_t i = 0; i < NUM_POINTS; i++)
    {
        if (chain_roots[i] == chain_root)
        {
            chain_roots[i] = 0;
            set_point(i, pointType::EMPTY);
        }
    }
    chain_liberties[chain_root] = 0;
    // we need this since set point adds liberties to adjacent chains on removal
}

void Board::update_chains(uint16_t idx)
{
    struct nbrs n = get_nbrs(idx);

    bool side = whose_turn();

    pointType oppositeSide = side ? pointType::WHITE : pointType::BLACK;
    pointType sameSide = side ? pointType::BLACK : pointType::WHITE;

    uint16_t num_same_color = ((side ? n.black : n.white) & COUNT) >> 4U;

    std::array<int, 4> prev_chains{};
    uint16_t prev_chain_count = 0;
    for (uint16_t i = 0; i < 4; i++)
    {
        uint16_t neighbor = idx + directions[i];
        if (board[neighbor] == oppositeSide)
        {
            uint16_t chain_id = chain_roots[neighbor];
#if DEBUG
            assert(chain_roots[neighbor] != 0);
            assert(chain_liberties[chain_roots[neighbor]] != 0);
            assert(chain_sizes[chain_roots[neighbor]] != 0);
#endif
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

                chain_liberties[chain_id]--;
                if (chain_liberties[chain_id] == 0)
                {
                    capture_chain(chain_id);
                }
            }
        }
    }

    if (num_same_color == 0)
    {
        // new chain
        create_chain(idx);
    }

    else if (num_same_color == 1)
    {
        // extension
        for (uint16_t i = 0; i < 4; i++)
        {
            uint16_t neighbor = idx + directions[i];
            if (board[neighbor] == sameSide)
            {
                extend_chain(idx, neighbor);
                break;
            }
        }
    }

    else
    {
        // merger or extension

        std::array<uint16_t, 4> same_color_chains{};
        uint16_t same_color_count = 0;
        for (uint16_t i = 0; i < 4; i++)
        {
            uint16_t neighbor = idx + directions[i];
            if (board[neighbor] == sameSide)
            {
                uint16_t chain_id = chain_roots[neighbor];

                bool dupl = false;
                for (uint16_t x = 0; x < same_color_count; x++)
                {
                    if (same_color_chains[x] == chain_id)
                    {
                        dupl = true;
                        break;
                    }
                }

                if (!dupl)
                {
                    same_color_chains[same_color_count] = chain_id;
                    same_color_count++;
                }
            }
        }

#if DEBUG
        assert(same_color_count > 0);
#endif

        if (same_color_count == 1)
        {
            extend_chain(idx, same_color_chains[0]);
        }
        else
        {
            merge_chains(same_color_chains, same_color_count, idx);
        }
    }
}