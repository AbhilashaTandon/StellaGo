#include "Agent.h"
#include "Config.h"
#include <iostream>

#define MIN_SCORE -32768
#define MAX_SCORE 32767

std::pair<uint16_t, int16_t> Agent::get_best_move(Board b, uint8_t depth)
{
    // todo: make hash table for best moves
    assert(depth > 0);
    std::pair<uint16_t, int16_t> results = alphabeta(Board(b), depth, MIN_SCORE, MAX_SCORE);
    return results;
}

std::pair<uint16_t, int16_t> Agent::alphabeta(Board b, uint8_t depth, int16_t alpha, int16_t beta)
{
    if (depth < 1)
    {
        return std::pair<uint16_t, int16_t>(0, b.stone_score());
    }
    int16_t value = 0;
    uint16_t best_move = PASS;
    if (b.whose_turn())
    {
        value = MIN_SCORE;
        // black to move
        for (int i = 0; i < NUM_POINTS; i++)
        {
            if (b.get_point(i) == pointType::EMPTY)
            {
                bool found_adj = false;
                // only play directly adjacent to stones or sides
                for (int d = 0; d < 4; d++)
                {
                    if (b.get_point(i + b.directions[d]) != EMPTY)
                    {
                        found_adj = true;
                        break;
                    }
                }
                if (!found_adj)
                {
                    continue;
                }
                Board copy = Board(b);
                if (copy.make_play(i))
                {
                    std::pair<uint16_t, int16_t> look_ahead = alphabeta(copy, depth - 1, alpha, beta);
                    if (look_ahead.second > value)
                    {
                        best_move = i;
                        value = look_ahead.second;
                    }
                    else if (look_ahead.second == value)
                    {
                        if (rand() % 10 == 0)
                        {
                            best_move = i;
                            value = look_ahead.second;
                        }
                    }
                    if (value >= beta)
                    {
                        break;
                    }
                    alpha = alpha > value ? alpha : value;
                    if (best_move != PASS)
                    {
                        Board copy_2 = Board(b);
                        assert(copy_2.make_play(best_move));
                    }
                }
            }
        }
    }
    else
    {
        value = MAX_SCORE;
        // black to move
        for (int i = 0; i < NUM_POINTS; i++)
        {
            if (b.get_point(i) == pointType::EMPTY)
            {
                bool found_adj = false;
                // only play directly adjacent to stones or sides
                for (int d = 0; d < 4; d++)
                {
                    if (b.get_point(i + b.directions[d]) != EMPTY)
                    {
                        found_adj = true;
                        break;
                    }
                }
                if (!found_adj)
                {
                    continue;
                }
                Board copy = Board(b);
                if (copy.make_play(i))
                {
                    std::pair<uint16_t, int16_t> look_ahead = alphabeta(copy, depth - 1, alpha, beta);
                    if (look_ahead.second < value)
                    {
                        best_move = i;
                        value = look_ahead.second;
                        // Board copy_2 = Board(b);
                        // assert(copy_2.make_play(best_move));
                    }
                    else if (look_ahead.second == value)
                    {
                        if (rand() % 10 == 0)
                        {
                            best_move = i;
                            value = look_ahead.second;
                        }
                    }
                    if (value <= alpha)
                    {
                        break;
                    }
                    beta = beta < value ? beta : value;
                }
            }
        }
    }
    return std::pair<uint16_t, int16_t>(best_move, value);
}

Agent::Agent() : b()
{
}

void Agent::play(uint8_t depth, uint16_t move_limit)
{
    bool white_pass = false;
    bool black_pass = false;
    for (uint16_t i = 0; i < move_limit; i++)
    {
        std::pair<uint16_t, int16_t> best_move = get_best_move(b, depth);
        if (best_move.first == PASS)
        {
            printf("PASS\n\n");
            if (b.whose_turn())
            {
                black_pass = true;
            }
            else
            {
                white_pass = true;
            }
            if (black_pass && white_pass)
            {
                int16_t score = b.stone_score();
                std::cout << "GAME OVER: " << ((score > 0) ? "BLACK" : "WHITE") << " wins!" << '\n';
                printf("Move: %u\tScore: %d\n", i, score);
                b.print_board();
                return;
            }
        }
        assert(b.make_play(best_move.first));
        printf("Move: %u\tScore: %d\n", i, b.stone_score());
        b.print_board();
    }
}
