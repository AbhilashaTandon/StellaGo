
#include "Board.h"

class Agent
{
public:
    std::pair<uint16_t, int16_t> get_best_move(Board b, uint8_t depth);
    std::pair<uint16_t, int16_t> alphabeta(Board b, uint8_t depth, int16_t alpha, int16_t beta);
    bool evaluate_move_white(Board b, int i, uint8_t depth, int16_t alpha, int16_t &beta, int16_t &value, uint16_t &best_move);
    bool evaluate_move_black(Board b, int i, uint8_t depth, int16_t &alpha, int16_t beta, int16_t &value, uint16_t &best_move);

    bool no_legal_moves(Board b);
    Agent();

    void play(uint8_t depth, uint16_t move_limit);

protected:
    Board b;
};