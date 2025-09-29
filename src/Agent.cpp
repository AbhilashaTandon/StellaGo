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
        return std::pair<uint16_t, int16_t>(0, b.score());
    }
    int16_t value = 0;
    uint16_t best_move = PASS;
    value = b.whose_turn() ? MIN_SCORE : MAX_SCORE;
    for (int idx = 0; idx < 361; idx++)
    {
        int i = optimal_move_checking_order[idx];
        if (b.get_point(i) == pointType::EMPTY)
        {
            if (b.whose_turn())
            {
                if (evaluate_move_black(b, i, depth, alpha, beta, value, best_move))
                {
                    break;
                }
            }
            else
            {
                if (evaluate_move_white(b, i, depth, alpha, beta, value, best_move))
                {
                    break;
                }
            }
        }
    }

    return std::pair<uint16_t, int16_t>(best_move, value);
}

bool Agent::evaluate_move_white(Board b, int i, uint8_t depth, int16_t alpha, int16_t &beta, int16_t &value, uint16_t &best_move)
{
    if (b.make_play(i))
    {
        std::pair<uint16_t, int16_t> look_ahead = alphabeta(b, depth - 1, alpha, beta);
        int score =
            look_ahead.second;
        if (score < value)
        {
            best_move = i;
            value = score;
        }
        if (value <= alpha)
        {
            {
                return true;
            };
        }
        beta = beta < value ? beta : value;
    }
    return false;
}

bool Agent::evaluate_move_black(Board b, int i, uint8_t depth, int16_t &alpha, int16_t beta, int16_t &value, uint16_t &best_move)
{
    if (b.make_play(i))
    {
        std::pair<uint16_t, int16_t> look_ahead = alphabeta(b, depth - 1, alpha, beta);
        int score =
            look_ahead.second;
        if (score > value)
        {
            best_move = i;
            value = score;
        }
        if (value >= beta)
        {
            {
                return true;
            };
        }
        alpha = alpha > value ? alpha : value;
    }
    return false;
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
                int16_t score = b.score();
                std::cout << "GAME OVER: " << ((score > 0) ? "BLACK" : "WHITE") << " wins!" << '\n';
                printf("Move: %u\tScore: %d\n", i, score);
                b.print_board();
                return;
            }
        }
        assert(b.make_play(best_move.first));
        printf("Move: %u\tScore: %d\n", i, b.score());
        b.print_board();
    }
}
