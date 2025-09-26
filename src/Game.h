#ifndef GAME_H
#define GAME_H

#include "Board.h"
#include <vector>
#include <unordered_map>

class Game
{
public:
    Game(int boardsize);
    bool make_play(int x, int y);
    bool whose_turn();
    int get_play_count();
    void print_board();
    void check_for_errors();

private:
    Board b;
    bool check_play(int x, int y);
    std::vector<int> chains;
    std::unordered_map<int, int> chain_liberties;
    void create_chain(int x, int y, bool color);
    void extend_chain(int x, int y, int chain_id);
    void merge_chains(std::vector<int> chain_ids, int x, int y);
    bool is_liberty_of_chain(std::vector<int> &chain_ids, int i, int board_pos);
    void capture_chain(int chain_id);
    int chain_is_neighbor(int idx, int chain_id, int excluded_point); // if chain neighbors point
    std::pair<int, int> ko;
    int play_count;
    int black_chain_ctr;
    int white_chain_ctr;
    int boardsize;
    std::vector<int> get_neighboring_chains(int x, int y);
    std::vector<int> get_neighboring_chains(int board_pos);
    void update_chains(int x, int y);
};

#endif