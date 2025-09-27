#include "Game.h"
#include "Board.h"
#include <algorithm>
#include <cstdio>
#include <iostream>

Game::Game(int board_size) : b(board_size)
{
    play_count = 0;
    black_chain_ctr = 1;
    white_chain_ctr = -1;
    this->chains = std::vector<int>((board_size + 2) * (board_size + 2), 0);
    boardsize = board_size;
    black_ko_hash = 0;
    white_ko_hash = 0;
}

Game::Game(const Game &g) : b(g.b)
{
    this->chains = g.chains;
    this->chain_liberties = g.chain_liberties;
    this->ko = g.ko;
    this->play_count = g.play_count;
    this->black_chain_ctr = g.black_chain_ctr;
    this->white_chain_ctr = g.white_chain_ctr;
    this->boardsize = g.boardsize;
    this->black_ko_hash = g.black_ko_hash;
    this->white_ko_hash = g.white_ko_hash;
}

bool Game::make_play(int x, int y)
{
    bool color_to_move = whose_turn();
    int idx = b.coords_to_idx(x, y);

    if (check_play(idx))
    {
        update_chains(idx);
        b.set_point(idx, color_to_move ? pointType::BLACK : pointType::WHITE);
        play_count++;
        if (color_to_move)
        {
            black_ko_hash = b.get_hash();
        }
        else
        {
            white_ko_hash = b.get_hash();
        }
        return true;
    }
    return false;
}

bool Game::whose_turn()
{
    return (play_count & 1) == 0;
}

int Game::get_play_count()
{
    return play_count;
}

void Game::check_for_errors()
{
    for (int i = 0; i < (boardsize + 2); i++)
    {
        for (int j = 0; j < (boardsize + 2); j++)
        {
            int board_pos = i * (boardsize + 2) + j;
            switch (b.get_point(i * (boardsize + 2) + j))
            {
            case pointType::BLACK:
            case pointType::WHITE:
                assert(chains[board_pos] != 0);
                assert(chain_liberties[chains[board_pos]] > 0);
                break;
            default:
                break;
            }
        }
    }

    for (std::pair<int, int> chain : chain_liberties)
    {
        int num_liberties = 0;
        for (int i = 0; i < (boardsize + 2) * (boardsize + 2); i++)
        {
            if (b.get_point(i) == pointType::EMPTY)
            {
                if (chain_is_neighbor(i, chain.first, 0))
                {
                    num_liberties++;
                }
            }
        }
        assert(num_liberties == chain.second);
    }
}

void Game::print_board()
{

    b.print_board();

    std::cout << std::endl;
    std::cout << "Chain Indices" << std::endl;

    for (int i = 0; i < (boardsize + 2); i++)
    {
        for (int j = 0; j < (boardsize + 2); j++)
        {
            int board_pos = i * (boardsize + 2) + j;
            switch (b.get_point(i * (boardsize + 2) + j))
            {
            case pointType::BLANK:
                std::cout << " # ";
                break;
            case pointType::EMPTY:
                std::cout << "   ";
                break;
            case pointType::BLACK:
            case pointType::WHITE:
                printf("%3d", chains[board_pos]);
                break;
            }
        }

        std::cout << std::endl;
    }

    std::cout << std::endl;
    std::cout << "Chain Liberties" << std::endl;

    for (int i = 0; i < (boardsize + 2); i++)
    {
        for (int j = 0; j < (boardsize + 2); j++)
        {
            int board_pos = i * (boardsize + 2) + j;
            switch (b.get_point(i * (boardsize + 2) + j))
            {
            case pointType::BLANK:
                std::cout << " # ";
                break;
            case pointType::EMPTY:
                std::cout << "   ";
                break;
            case pointType::BLACK:
            case pointType::WHITE:
                printf("%3d", chain_liberties[chains[board_pos]]);
                break;
            }
        }

        std::cout << std::endl;
    }
    std::cout << std::endl;

    printf("Board hash: %lx\n", b.get_hash());
    printf("Black ko hash: %lx\n", black_ko_hash);
    printf("White ko hash: %lx\n", white_ko_hash);

    printf("Play Count %d, %s to move\n----------------\n", play_count, whose_turn() ? "black" : "white");
}

bool Game::check_play(int idx)
{
    bool color_to_move = whose_turn();

    struct nbrs neighbors = b.get_nbrs(idx);

    if (b.get_point(idx) != pointType::EMPTY)
    {
        return false;
    }

    // printf("%d %d ", idx);
    // printf("%d %d %d %d\n", (neighbors.liberties & COUNT) >> 4, (neighbors.black & COUNT) >> 4, (neighbors.white & COUNT) >> 4, (neighbors.edges & COUNT) >> 4);

    if ((neighbors.liberties & COUNT) == 0)
    {

        if (!check_if_suicide(idx, color_to_move))
        {
            return false;
        }
    }

    // ko checking

    Game copy = *this;
    copy.update_chains(idx);

    if (color_to_move)
    {
        // black to move
        copy.b.set_point(idx, pointType::BLACK);
        // printf("\n\nlast position: %lx, possible next pos: %lx\n\n", black_ko_hash, copy.b.get_hash());
        return black_ko_hash != copy.b.get_hash();
        // if hashes are different not a repeat (except in case of hash collision ig)
    }
    else
    {
        // white to move
        copy.b.set_point(idx, pointType::WHITE);
        // printf("\n\nlast position: %lx, possible next pos: %lx\n\n", white_ko_hash, copy.b.get_hash());
        return white_ko_hash != copy.b.get_hash();
        // if hashes are different not a repeat (except in case of hash collision ig)
    }

    // printf("has liberty");
    // return true;
}

bool Game::check_if_suicide(int idx, bool color_to_move)
{
    // check if move is suicide
    // if all neighboring opposite color chains have at least 2 liberties (one for stone to be added and another one for safety, they cant be captured)
    //
    // and no neighboring same color chain has at least 2 liberties (one for stone to be added, it can be captured) then it is suicide

    // if a neighboring opposite color chain has < 2 liberties or a neighboring same color chain has > 1 liberties then it is not suicide

    std::vector<int> chain_neighbors = get_neighboring_chains(idx);

    if (chain_neighbors.size() == 0)
    {
        return true;
    }

    for (int chain_id : chain_neighbors)
    {
        if (color_to_move == (chain_id > 0))
        { // same color chain
            if (chain_liberties[chain_id] > 1)
            {
                // printf("extension to alive chain");
                return true;
            }
        }
        else
        // diff color chain
        {
            if (chain_liberties[chain_id] < 2)
            {
                // printf("capture of dead chain");
                return true;
            }
        }
    }

    return false;
}

void Game::create_chain(int board_pos, bool color)
{
#if DEBUG
    assert(chains[board_pos] == 0);
#endif
    chains[board_pos] = color ? black_chain_ctr : white_chain_ctr;
    chain_liberties[chains[board_pos]] = b.get_liberties(board_pos);
    if (color)
    {
        black_chain_ctr++;
    }
    else
    {
        white_chain_ctr--;
    }
}

int Game::chain_is_neighbor(int board_pos, int chain_id, int excluded_point)
{
    // for each liberty
    for (int j = 0; j < 4; j++)
    {
        int nbr_of_lib = (board_pos + b.directions[j]);
        if (nbr_of_lib == excluded_point)
        {
            continue;
        }

        if (chains[nbr_of_lib] == chain_id)
        {
            return true;
        }
    }
    return false;
}

int Game::chain_is_neighbor(int board_pos, int chain_id, int excluded_point, int merge_point)
{
    // for each liberty
    for (int j = 0; j < 4; j++)
    {
        int nbr_of_lib = (board_pos + b.directions[j]);
        if (nbr_of_lib == excluded_point)
        {
            continue;
        }

        if (nbr_of_lib == merge_point)
        {
            return true;
        }

        if (chains[nbr_of_lib] == chain_id)
        {
            return true;
        }
    }
    return false;
}

std::vector<int> Game::get_neighboring_chains(int board_pos)
{
    std::vector<int> neighbors = std::vector<int>();
    for (int i = 0; i < 4; i++)
    {
        int chain_id = chains[board_pos + b.directions[i]];
        if (chain_id != 0)
        {
            if (std::find(neighbors.begin(), neighbors.end(), chain_id) == neighbors.end())
            // i should replace this with a set later for efficiency
            {
                neighbors.push_back(chains[board_pos + b.directions[i]]);
            }
        }
    }
    return neighbors;
}

void Game::update_chains(int idx)
{
    struct nbrs n = b.get_nbrs(idx);

    bool side = whose_turn();

    // printf("\n\n%d\n\n", side);

    int num_same_color = ((side ? n.black : n.white) & COUNT) >> 4;

    // printf("\n\nBLACK: %x\tWHITE: %x\tEDGE: %x\tLIB: %x\n\n", n.black, n.white, n.edges, n.liberties);

    // printf("%d", (side ? n.black : n.white) & COUNT);

    // printf("%x %x", n.black, n.white);

    std::vector<int> neighboring_chains = get_neighboring_chains(idx);

    for (int chain_id : neighboring_chains)
    {
        if (side != (chain_id > 0)) // diff color chain
        {
            chain_liberties[chain_id]--;
            if (chain_liberties[chain_id] == 0)
            {
                capture_chain(chain_id);
            }
        }
    }

    if (num_same_color == 0)
    {
        // new chain
        // printf("new chain");
        create_chain(idx, side);
    }

    else if (num_same_color == 1)
    {
        // extension
        for (int chain_id : neighboring_chains)
        {

            if (side == (chain_id > 0))
            {
                // printf("extension");
                extend_chain(idx, chain_id);
                break;
            }
        }
    }

    else
    {
        // merger or extension

        // printf("merger or extension");

        std::vector<int> same_color_chains = std::vector<int>();

        for (int chain_id : neighboring_chains)
        {
            if (side == (chain_id > 0))
            {
                same_color_chains.push_back(chain_id);
            }
        }

#if DEBUG
        assert(same_color_chains.size() > 0);
#endif

        if (same_color_chains.size() == 1)
        {
            // printf("extension");
            extend_chain(idx, same_color_chains[0]);
        }
        else if (same_color_chains.size() == 2)
        {
            // printf("merger");
            merge_two_chains(same_color_chains[0], same_color_chains[1], idx);
        }
        else if (same_color_chains.size() > 2)
        {
            merge_chains(same_color_chains, idx);
        }
    }
}

void Game::extend_chain(int board_pos, int chain_id)

{
    // int board_pos = b.coords_to_idx(idx);
#if DEBUG
    assert(chains[board_pos] == 0);
#endif
    chains[board_pos] = chain_id;
    chain_liberties[chain_id]--;

    uint8_t libs = b.get_nbrs(board_pos).liberties;

    for (int i = 0; i < 4; i++)
    {
        if (libs & (1 << i))
        // if the new stone has liberties
        {
            if (!chain_is_neighbor(board_pos + b.directions[i], chain_id, board_pos))
            // if a liberty of the new stone doesn't border the chain already then add it for a new liberty
            {
                chain_liberties[chain_id]++;
            }
        }
    }

    chains[board_pos] = chain_id;
}

void Game::merge_chains(std::vector<int> chain_ids, int board_pos)

{
#if DEBUG
    assert(chains[board_pos] == 0);
    // merge point must be currently empty for a new stone to be placed there
    for (unsigned int x = 0; x < chain_ids.size(); x++)
    {
        for (unsigned int y = 0; y < x; y++)
        {
            assert(chain_ids[x] != chain_ids[y]);
            // no double ups
            assert((chain_ids[x] < 0) == ((chain_ids[y]) < 0));
            // chains must be same color
        }
    }
#endif

    // int board_pos = b.coords_to_idx(idx);

    int new_chain_id = *std::min_element(chain_ids.begin(), chain_ids.end());

    // update chain ids
    for (int i = 0; i < boardsize; i++)
    {
        for (int j = 0; j < boardsize; j++)
        {
            int idx = b.coords_to_idx(i, j);
            // for every point on the board
            if (b.get_point(idx) != pointType::EMPTY)
            {
                if (std::find(chain_ids.begin(), chain_ids.end(), chains[idx]) != chain_ids.end())
                {
                    chains[idx] = new_chain_id;
                    // update the board_pos of the chains
                }
            }
        }
    }

    // calculate num_liberties
    int new_libs = 0;
    for (int i = 0; i < boardsize; i++)
    {
        for (int j = 0; j < boardsize; j++)
        {
            int idx = b.coords_to_idx(i, j);
            if (idx == board_pos)
            {
                continue;
            }
            if (b.get_point(idx) == pointType::EMPTY)
            {
                if (chain_is_neighbor(idx, new_chain_id, 0, board_pos))
                {
                    // printf("\n%d %d\n", i, j);
                    new_libs++;
                }
            }
        }
    }

    for (int x : chain_ids)
    {
        chain_liberties.erase(x);
    }

    chains[board_pos] = new_chain_id;
    chain_liberties[new_chain_id] = new_libs;
}

void Game::merge_two_chains(int chain_id_1, int chain_id_2, int board_pos)
{
#if DEBUG
    assert(chains[board_pos] == 0);
    // merge point must be currently empty for a new stone to be placed there
    assert(chain_id_1 != chain_id_2);
    // no double ups
    assert((chain_id_1 < 0) == ((chain_id_2) < 0));
    // chains must be same color
#endif

    int new_chain_id = (chain_id_1 < chain_id_2) ? chain_id_1 : chain_id_2;
    int old_chain_id = (new_chain_id == chain_id_1) ? chain_id_2 : chain_id_1;

    // update chain ids
    for (int i = 0; i < boardsize; i++)
    {
        for (int j = 0; j < boardsize; j++)
        {
            int idx = b.coords_to_idx(i, j);
            // for every point on the board
            if (b.get_point(idx) != pointType::EMPTY)
            {
                if (chains[idx] == old_chain_id)
                {
                    chains[idx] = new_chain_id;
                    // update the board_pos of the chains
                }
            }
        }
    }

    // calculate num_liberties
    int new_libs = 0;
    for (int i = 0; i < boardsize; i++)
    {
        for (int j = 0; j < boardsize; j++)
        {
            int idx = b.coords_to_idx(i, j);
            if (idx == board_pos)
            {
                continue;
            }
            if (b.get_point(idx) == pointType::EMPTY)
            {
                if (chain_is_neighbor(idx, new_chain_id, 0, board_pos))
                {
                    // printf("\n%d %d\n", i, j);
                    new_libs++;
                }
            }
        }
    }

    chain_liberties.erase(old_chain_id);

    chains[board_pos] = new_chain_id;
    chain_liberties[new_chain_id] = new_libs;
}

bool Game::is_liberty_of_chain(std::vector<int> &chain_ids, int i, int board_pos)
{
    for (int x : chain_ids)
    {
        if (chain_is_neighbor(i, x, 0))
        // and borders our new merged chain
        {
            return true;
        }
    }

    for (int d = 0; d < 4; d++)
    // or borders the linking stone
    {
        if (i == (board_pos + b.directions[d]))
        {
            return true;
        }
    }
    return false;
}

void Game::capture_chain(int chain_id)
{
#if DEBUG
    assert(chain_liberties[chain_id] == 0);
#endif
    // make sure to decrement chain liberties before calling this method

    for (int i = 0; i < boardsize; i++)
    {
        for (int j = 0; j < boardsize; j++)
        {
            int idx = b.coords_to_idx(i, j);
            if (chains[idx] == chain_id)
            {
                chains[idx] = 0;
                b.set_point(idx, pointType::EMPTY);
                for (int chain_id : get_neighboring_chains(idx))
                {
                    chain_liberties[chain_id]++;
                }
            }
        }
    }

    chain_liberties.erase(chain_id);
}
