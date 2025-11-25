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

bool Board::no_legal_moves() const
{
    for (int i = 0; i < NUM_POINTS; i++)
    {
        if (i != EMPTY)
        {
            continue;
        }

        if (check_play(i))
        {
            return true;
        }
    }
    return false;
}

void Board::evaluate_game()
{
    float score = area_score();
    if (score > 0)
    {
        game_result = BLACK;
    }
    else if (score < 0)
    {
        game_result = WHITE;
    }
    else
    {
        game_result = BLANK;
    }
}

pointType Board::get_game_result() const
{
    return game_result;
}

bool Board::make_play(uint16_t idx)
{
    bool color_to_play = whose_turn();
    if (idx == PASS)
    {
        return pass_play(color_to_play);
    }

    black_passed = false;
    white_passed = false;

    if (idx == RESIGN)
    {
        if (color_to_play)
        {
            game_result = WHITE;
            return true;
        }
        else
        {
            game_result = BLACK;
            return true;
        }
    }

#if DEBUG
    assert(chain_roots[idx] == 0);
#endif

    if (check_play(idx))
    {
        // print_board();
        update_chains(idx);
        set_point(idx, color_to_play ? pointType::BLACK : pointType::WHITE);
        play_count++;
        if (color_to_play)
        {
            black_ko_hash = get_hash();
        }
        else
        {
            white_ko_hash = get_hash();
        }
#if DEBUG

        check_for_errors();
#endif
#if VERBOSE
        print_board();
#endif
        return true;
    }
    else
    {
        if (no_legal_moves())
        {
            // if no legal moves we auto pass
            pass_play(color_to_play);
            return true;
        }
    }
    return false;
}

bool Board::pass_play(bool color_to_play)
{
    if (color_to_play)
    {
        black_passed = true;
    }
    else
    {
        white_passed = true;
    }
    play_count++;
    if (black_passed && white_passed)
    {
        evaluate_game();
        return true;
    }
    return true;
}

bool Board::whose_turn() const
{
    return (play_count & 1) == 0;
}