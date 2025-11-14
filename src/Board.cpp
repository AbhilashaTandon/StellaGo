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
}

uint16_t Board::get_liberties(uint16_t idx) const
{
    uint16_t num_liberties = 0;
    for (uint16_t i = 0; i < 4; i++)
    {
        if (board[idx + directions[i]] == pointType::EMPTY)
        {
            num_liberties++;
        }
    }
    return num_liberties;
}

void Board::print_board() const
{
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
                printf("%3d", chain_liberties[i * (BOARD_SIZE + 2) + j]);
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

bool Board::make_play(uint16_t idx)
{
    if (idx == PASS)
    {
        play_count++;
        return true;
    }
    bool color_to_move = whose_turn();

#if DEBUG
    assert(chain_roots[idx] == 0);
#endif

    if (check_play(idx))
    {
        // print_board();
        update_chains(idx);
        set_point(idx, color_to_move ? pointType::BLACK : pointType::WHITE);
        play_count++;
        if (color_to_move)
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
    return false;
}

bool Board::whose_turn() const
{
    return (play_count & 1) == 0;
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
            if (chain_liberties[chain_roots[i]] == 0)
            {
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
                        liberties_check[chain_roots[i + directions[d]]]++;
                    }
                }
            }
            break;
        }
    }
    for (uint16_t i = 0; i < NUM_POINTS; i++)
    {
        // std::cout << i << " " << liberties_check[i] << " " << chain_liberties[i] << '\n';
        // std::cout << i << " " << sizes_check[i] << " " << chain_sizes[i] << '\n';
        if (liberties_check[i] != chain_liberties[i])
        {
            assert(false);
        }
        if (sizes_check[i] != chain_sizes[i])
        {

            assert(false);
        }
    }
}
