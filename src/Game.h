#ifndef GAME_H
#define GAME_H

#include "Board.h"
#include <vector>
#include <unordered_map>

class Game
{
public:
    Game(int board_size);
    bool make_play(int x, int y);
    bool whose_turn();
    int get_play_count();

private:
    Board b;
    bool check_play(int x, int y);
    std::vector<int> chains;
    std::unordered_map<int, int> chain_liberties;
    void create_chain(int x, int y, bool color);
    void extend_chain(int x, int y, int chain_id);
    void merge_chains(int chain_id_1, int chain_id_2);
    void capture_chain(int chain_id);
    std::pair<int, int> ko;
    int play_count;
    int chain_count;
};

#endif