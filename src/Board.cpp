#include <iostream>
#include <random>
#include <cmath>

#include "Board.h"

Board::Board()
{
    for (uint16_t i = 0; i < NUM_POINTS; i++)
    {
        this->board[i] = pointType::EMPTY;
    }

    for (uint16_t i = 0; i < BOARD_SIZE + 1; i++)
    {
        // add in edges of board
        board[i + 1] = pointType::BLANK;
        board[(i) * (BOARD_SIZE + 2)] = pointType::BLANK;
        board[(BOARD_SIZE + 1) * (BOARD_SIZE + 2) + i] = pointType::BLANK;
        board[(i + 1) * (BOARD_SIZE + 2) + (BOARD_SIZE + 1)] = pointType::BLANK;
    }

    this->directions = {-BOARD_SIZE - 2, -1, BOARD_SIZE + 2, 1};
    this->diagonals = {-BOARD_SIZE - 3, -BOARD_SIZE - 1, BOARD_SIZE + 1, BOARD_SIZE + 3};

    zobrist = 0; // empty board state

    // set up random 64 bitstring generator
    std::random_device rd;

    std::mt19937_64 e2(rd());

    std::uniform_int_distribution<long long int> dist(std::llround(std::pow(2, 61)), std::llround(std::pow(2, 62)));

    for (uint16_t i = 0; i < NUM_POINTS; i++)
    {
        zobrist_hashes_black[i] = dist(e2);
        zobrist_hashes_white[i] = dist(e2);
    }

    black_count = 0;
    white_count = 0;
    empty_count = (BOARD_SIZE) * (BOARD_SIZE);

    play_count = 0;
    black_ko_hash = 0;
    white_ko_hash = 0;
    game_result = EMPTY;
}

void Board::print_board() const
{
    // TODO: put last move in parentheses to make it easier to track what's going on
    for (uint16_t i = 0; i < (BOARD_SIZE + 2); i++)
    {
        for (uint16_t j = 0; j < (BOARD_SIZE + 2); j++)
        {
            switch (board[i * (BOARD_SIZE + 2) + j])
            {
            case pointType::BLANK:
                std::cout << "# ";
                break;
            case pointType::EMPTY:
                if (is_eye(i * (BOARD_SIZE + 2) + j))
                {
                    std::cout << "* ";
                    break;
                }
                std::cout << "  ";
                break;
            case pointType::BLACK:
                std::cout << "○ ";
                break;
            case pointType::WHITE:
                std::cout << "● ";
                break;
            }
        }

        std::cout << std::endl;
    }
    std::cout << std::endl;
#if DEBUG
#if VERBOSE
    std::cout << "Chain Roots" << std::endl;
    std::cout << std::endl;

    for (uint16_t i = 0; i < (BOARD_SIZE + 2); i++)
    {
        for (uint16_t j = 0; j < (BOARD_SIZE + 2); j++)
        {
            switch (board[i * (BOARD_SIZE + 2) + j])
            {
            case pointType::BLANK:
                std::cout << " # ";
                break;
            case pointType::EMPTY:
                std::cout << "   ";
                break;
            case pointType::BLACK:
            case pointType::WHITE:
                printf("%3d", chain_roots[i * (BOARD_SIZE + 2) + j]);
                break;
            }
        }

        std::cout << std::endl;
    }

    std::cout << std::endl;
    std::cout << "Chain Liberties" << std::endl;
    std::cout << std::endl;

    for (uint16_t i = 0; i < (BOARD_SIZE + 2); i++)
    {
        for (uint16_t j = 0; j < (BOARD_SIZE + 2); j++)
        {
            switch (board[i * (BOARD_SIZE + 2) + j])
            {
            case pointType::BLANK:
                std::cout << " # ";
                break;
            case pointType::EMPTY:
                std::cout << "   ";
                break;
            case pointType::BLACK:
            case pointType::WHITE:
                printf("%3d", chain_liberty_counts[i * (BOARD_SIZE + 2) + j]);
                break;
            }
        }

        std::cout << std::endl;
    }

    std::cout << std::endl;
    std::cout << "Chain Sizes" << std::endl;
    std::cout << std::endl;

    for (uint16_t i = 0; i < (BOARD_SIZE + 2); i++)
    {
        for (uint16_t j = 0; j < (BOARD_SIZE + 2); j++)
        {
            switch (board[i * (BOARD_SIZE + 2) + j])
            {
            case pointType::BLANK:
                std::cout << " # ";
                break;
            case pointType::EMPTY:
                std::cout << "   ";
                break;
            case pointType::BLACK:
            case pointType::WHITE:
                printf("%3d", chain_sizes[i * (BOARD_SIZE + 2) + j]);
                break;
            }
        }
        std::cout << std::endl;
    }
#endif
#endif
}

uint64_t Board::get_hash() const
{
    return zobrist;
}

uint16_t Board::get_play_count() const
{
    return play_count;
}

void Board::check_for_errors() const
{
    std::array<uint16_t, NUM_POINTS> liberties_check{};
    std::array<uint16_t, NUM_POINTS> sizes_check{};
    for (uint16_t i = 0; i < NUM_POINTS; i++)
    {
        switch (board[i])
        {
        case pointType::BLANK:
            break;
        case pointType::BLACK:
        case pointType::WHITE:
            sizes_check[chain_roots[i]]++;

            assert(chain_roots[i] != 0);
            if (chain_liberty_counts[chain_roots[i]] == 0)
            {

                std::pair<int, int> loc = this->idx_to_coords(i);
                std::cout << loc.first << " " << loc.second << "\n";
                assert(false);
            }
            assert(chain_sizes[chain_roots[i]] != 0);
            break;
        case pointType::EMPTY:
            std::array<uint16_t, 4> past_chains{};
            uint16_t num_past_chains = 0;

            for (uint16_t d = 0; d < 4; d++)
            {
                uint16_t chain_id = chain_roots[i + directions[d]];
                if (chain_id != 0)
                {
                    bool dupl = false;
                    for (uint16_t x = 0; x < num_past_chains; x++)
                    {
                        if (past_chains[x] == chain_id)
                        {
                            dupl = true;
                            break;
                        }
                    }

                    if (!dupl)
                    {
                        past_chains[num_past_chains] = chain_id;
                        num_past_chains++;
                        // std::cout << i << " " << chain_roots[i + directions[d]] << '\n';
                        // assert(chain_liberty_locations[chain_id][i + directions[d]]);
                        liberties_check[chain_roots[i + directions[d]]]++;
                    }
                }
            }
            break;
        }
    }
    for (uint16_t i = 0; i < NUM_POINTS; i++)
    {
        // std::cout << i << " " << liberties_check[i] << " " << chain_liberty_counts[i] << '\n';
        // std::cout << i << " " << sizes_check[i] << " " << chain_sizes[i] << '\n';
        if (liberties_check[i] != chain_liberty_counts[i])
        {
            assert(false);
        }
        if (sizes_check[i] != chain_sizes[i])
        {

            assert(false);
        }
    }
}

uint16_t Board::coords_to_idx(uint16_t x, uint16_t y) const
{
#if DEBUG
    assert(x >= 0 && x < BOARD_SIZE);
    assert(y >= 0 && y < BOARD_SIZE);
#endif
    return (BOARD_SIZE + 2) * (y + 1) + x + 1;
}
std::pair<int, int> Board::idx_to_coords(uint16_t idx) const
{
#if DEBUG
    assert(idx >= 0 && (unsigned int)idx < board.size());
#endif
    return std::pair<int, int>(idx / (BOARD_SIZE + 2) - 1, idx % (BOARD_SIZE + 2) - 1);
}

uint8_t Board::is_eye(uint16_t idx) const
{
    if (board[idx] != EMPTY)
    {
        return false;
    }
    nbrs n = get_nbrs(idx);
    uint8_t color = 0;
    if ((((n.black & COUNT) >> 4) + ((n.edges & COUNT) >> 4)) == 4)
    {
        color = BLACK;
    }
    else if ((((n.white & COUNT) >> 4) + ((n.edges & COUNT) >> 4)) == 4)
    {
        color = WHITE;
    }
    else
    {
        // std::printf("lacks direct neighbors %d %d %d\n", n.edges & COUNT, n.black, n.white);
        return 0;
    }

    // check for at least 2 diagonals

    int num_diagonals = 0;
    for (int i = 0; i < 4; i++)
    {
        if (board[idx + diagonals[i]] == color || board[idx + diagonals[i]] == BLANK)
        {
            num_diagonals++;
        }
        if (num_diagonals >= 2)
        {
            // this includes false eyes
            return color;
        }
    }

    // std::cout << "lacks diagonal neighbors";
    return 0;
}