#include <iostream>
#include <random>
#include <cmath>

#include "Board.h"

bool Board::check_play(uint16_t idx) const
{
    if (board[idx] != pointType::EMPTY)
    {
        return false;
    }

    if (is_suicide(idx))
    {
        return false;
    }

    bool color_to_play = whose_turn();

    // ko checking

    Board copy = *this;
    copy.update_chains(idx);

    if (color_to_play)
    {
        // black to play
        copy.set_point(idx, pointType::BLACK);
        return black_ko_hash != copy.get_hash();
        // if hashes are different not a repeat (except in case of hash collision ig)
    }
    else
    {
        // white to play
        copy.set_point(idx, pointType::WHITE);
        return white_ko_hash != copy.get_hash();
        // if hashes are different not a repeat (except in case of hash collision ig)
    }
}

bool Board::is_suicide(uint16_t idx) const
{
    // check if play is suicide
    // if all neighboring opposite color chains have at least 2 liberties (one for stone to be added and another one for safety, they cant be captured)
    //
    // and no neighboring same color chain has at least 2 liberties (one for stone to be added, it can be captured) then it is suicide

    // if a neighboring opposite color chain has < 2 liberties or a neighboring same color chain has > 1 liberties then it is not suicide
    bool color_to_play = whose_turn();

    for (uint16_t i = 0; i < 4; i++)
    {
        uint16_t chain_id = chain_roots[idx + directions[i]];
        switch (board[idx + directions[i]])
        {
        case pointType::BLANK:
            break;
        case pointType::EMPTY:
            // if there's a liberty its not suicide
            return false;
        case pointType::BLACK:
            if (color_to_play)
            {
                // same color chain
                if (chain_liberty_counts[chain_id] > 1)
                {
                    return false;
                }
            }
            else
            // diff color chain
            {
                if (chain_liberty_counts[chain_id] < 2)
                {
                    return false;
                }
            }
            break;
        case pointType::WHITE:
            if (!color_to_play)
            {
                // same color chain
                if (chain_liberty_counts[chain_id] > 1)
                {
                    return false;
                }
            }
            else
            // diff color chain
            {
                if (chain_liberty_counts[chain_id] < 2)
                {
                    return false;
                }
            }
            break;
        }
    }

    return true;
}