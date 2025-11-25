
#include "Board.h"

class Agent
{
public:
    std::pair<uint16_t, int16_t> get_best_play(uint8_t depth);
    std::pair<uint16_t, int16_t> alphabeta(Board b, uint8_t depth, int16_t alpha, int16_t beta);
    bool evaluate_play_white(Board b, int i, uint8_t depth, int16_t alpha, int16_t &beta, int16_t &value, uint16_t &best_play);
    bool evaluate_play_black(Board b, int i, uint8_t depth, int16_t &alpha, int16_t beta, int16_t &value, uint16_t &best_play);

    bool no_legal_plays(Board b);
    Agent();

    void play(uint8_t depth, uint16_t play_limit);
    // auto plays against itself

    bool make_play(int i);
    bool play_best_play(uint8_t depth);

protected:
    Board b;
};