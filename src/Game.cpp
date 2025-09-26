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
}

bool Game::make_play(int x, int y)
{
    bool color_to_move = (play_count & 1) == 0;

    if (check_play(x, y))
    {
        update_chains(x, y);
        b.set_point(x, y, color_to_move ? pointType::BLACK : pointType::WHITE);
        play_count++;
        return true;
    }
    return false;
}

bool Game::whose_turn()
{
    return play_count & 1;
}

int Game::get_play_count()
{
    return play_count;
}

void Game::print_board()
{
    printf("Play Count %d, %s to move\n\n", play_count - 1, ((play_count & 1) == 0) ? "black" : "white");

    b.print_board();

    std::cout << std::endl;
    std::cout << "Chain Indices" << std::endl;

    for (int i = 0; i < (boardsize + 2); i++)
    {
        for (int j = 0; j < (boardsize + 2); j++)
        {
            int idx = i * (boardsize + 2) + j;
            switch (b.get_point(i * (boardsize + 2) + j))
            {
            case pointType::BLANK:
                std::cout << "  #";
                break;
            case pointType::EMPTY:
                std::cout << "   ";
                break;
            case pointType::BLACK:
            case pointType::WHITE:
                printf("%3d", chains[idx]);
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
            int idx = i * (boardsize + 2) + j;
            switch (b.get_point(i * (boardsize + 2) + j))
            {
            case pointType::BLANK:
                std::cout << "# ";
                break;
            case pointType::EMPTY:
                std::cout << "  ";
                break;
            case pointType::BLACK:
            case pointType::WHITE:
                printf("%d ", chain_liberties[chains[idx]]);
                break;
            }
        }

        std::cout << std::endl;
    }
    std::cout << std::endl;
}

bool Game::check_play(int x, int y)
{
    bool color_to_move = (play_count & 1) == 0;

    struct nbrs neighbors = b.get_nbrs(x, y);

    if (b.get_point(x, y) != pointType::EMPTY)
    {
        return false;
    }

    if ((neighbors.liberties & COUNT) == 0)
    {
        // check if move is suicide
        // if all neighboring opposite color chains have at least 2 liberties (one for stone to be added and another one for safety, they cant be captured)
        //
        // and no neighboring same color chain has at least 2 liberties (one for stone to be added, it can be captured) then it is suicide

        // if a neighboring opposite color chain has < 2 liberties or a neighboring same color chain has > 1 liberties then it is not suicide

        std::vector<int> chain_neighbors = get_neighboring_chains(x, y);

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
                    return true;
                }
            }
            else
            // diff color chain
            {
                if (chain_liberties[chain_id] < 2)
                {
                    return true;
                }
            }
        }

        return false;
    }

    return true;
}

void Game::create_chain(int x, int y, bool color)
{
    int idx = b.coords_to_idx(x, y);
    assert(chains[idx] == 0);
    chains[idx] = color ? black_chain_ctr : white_chain_ctr;
    chain_liberties[chains[idx]] = b.get_liberties(x, y);
    if (color)
    {
        black_chain_ctr++;
    }
    else
    {
        white_chain_ctr--;
    }

    for (int chain_id : get_neighboring_chains(x, y))
    {
        chain_liberties[chain_id] = chain_liberties[chain_id] - 1;
    }
}

int Game::chain_is_neighbor(int idx, int chain_id, int excluded_point)
{
    // for each liberty
    for (int j = 0; j < 4; j++)
    {
        int nbr_of_lib = (idx + b.directions[j]);
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

std::vector<int> Game::get_neighboring_chains(int x, int y)
{
    int idx = b.coords_to_idx(x, y);
    std::vector<int> neighbors = std::vector<int>();
    for (int i = 0; i < 4; i++)
    {
        int chain_id = chains[idx + b.directions[i]];
        if (chain_id != 0)
        {
            if (std::find(neighbors.begin(), neighbors.end(), chain_id) == neighbors.end())
            // i should replace this with a set later for efficiency
            {
                neighbors.push_back(chains[idx + b.directions[i]]);
            }
        }
    }
    return neighbors;
}

void Game::update_chains(int x, int y)
{
    struct nbrs n = b.get_nbrs(x, y);

    bool side = (play_count & 1) == 0;

    int num_same_color = ((side ? n.black : n.white) & COUNT) >> 4;

    std::vector<int> neighboring_chains = get_neighboring_chains(x, y);

    if (num_same_color == 0)
    {
        // new chain
        create_chain(x, y, side);
    }

    else if (num_same_color == 1)
    {
        // extension
        for (int chain_id : neighboring_chains)
        {

            if (side == (chain_id > 0))
            {
                extend_chain(x, y, chain_id);
                break;
            }
        }
    }

    else
    {
        // merger or extension

        std::vector<int> same_color_chains = std::vector<int>();

        for (int chain_id : neighboring_chains)
        {
            if (side == (chain_id > 0))
            {
                same_color_chains.push_back(chain_id);
            }
        }

        assert(same_color_chains.size() > 0);

        if (same_color_chains.size() == 1)
        {
            extend_chain(x, y, same_color_chains[0]);
        }
        else if (same_color_chains.size() > 1)
        {
            merge_chains(same_color_chains, x, y);
        }
    }

    for (int chain_id : neighboring_chains)
    {
        if (side != (chain_id > 0))
        {
            // diff color chain
            chain_liberties[chain_id]--;
            if (chain_liberties[chain_id] == 0)
            {
                capture_chain(chain_id);
            }
        }
    }
}

void Game::extend_chain(int x, int y, int chain_id)
{
    int idx = b.coords_to_idx(x, y);
    assert(chains[idx] == 0);
    chains[idx] = chain_id;
    chain_liberties[chain_id]--;

    uint8_t libs = b.get_nbrs(x, y).liberties;

    for (int i = 0; i < 4; i++)
    {
        if (libs & (1 << i))
        // if the new stone has liberties
        {
            if (!chain_is_neighbor(idx + b.directions[i], chain_id, idx))
            // if a liberty of the new stone doesn't border the chain already then add it for a new liberty
            {
                chain_liberties[chain_id]++;
            }
        }
    }

    chains[idx] = chain_id;
}

void Game::merge_chains(std::vector<int> chain_ids, int x, int y)
{
    assert(chains[b.coords_to_idx(x, y)] == 0);
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

    int new_chain_id = *std::min_element(chain_ids.begin(), chain_ids.end());
    int new_libs = 0;

    for (int i = 0; i < (boardsize + 2) * (boardsize + 2); i++)
    {
        // for every point on the board
        if (std::find(chain_ids.begin(), chain_ids.end(), chains[i]) != chain_ids.end())
        {
            chains[i] = new_chain_id;
            // update the idx of the chains
        }
        else if (b.get_point(i) == pointType::EMPTY)
        // if its empty
        {
            for (int x : chain_ids)
            {
                if (chain_is_neighbor(i, x, 0))
                // and borders our new merged chain
                {
                    new_libs++;
                    // add a liberty
                    break;
                }
            }
        }
    }

    for (int x : chain_ids)
    {
        chain_liberties.erase(x);
    }

    chains[b.coords_to_idx(x, y)] = new_chain_id;
    chain_liberties[new_chain_id] = new_libs;
}

void Game::capture_chain(int chain_id)
{
    assert(chain_liberties[chain_id] == 0);
    // make sure to decrement chain liberties before calling this method

    for (int i = 0; i < (boardsize + 2) * (boardsize + 2); i++)
    {
        if (chains[i] == chain_id)
        {
            chains[i] = 0;
        }
    }

    chain_liberties.erase(chain_id);
}
