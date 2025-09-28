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

Board::Board(const Board &b)
{
    this->board = b.board;
    this->directions = b.directions;
    this->zobrist = b.zobrist;
    this->zobrist_hashes_black = b.zobrist_hashes_black;
    this->zobrist_hashes_white = b.zobrist_hashes_white;
    this->black_count = b.black_count;
    this->white_count = b.white_count;
    this->empty_count = b.empty_count;
    this->black_ko_hash = b.black_ko_hash;
    this->white_ko_hash = b.white_ko_hash;
    this->chain_liberties = b.chain_liberties;
    this->chain_roots = b.chain_roots;
    this->chain_sizes = b.chain_sizes;
}

uint16_t Board::coords_to_idx(uint16_t x, uint16_t y)
{
#if DEBUG
    assert(x >= 0 && x < BOARD_SIZE);
    assert(y >= 0 && y < BOARD_SIZE);
#endif
    return (BOARD_SIZE + 2) * (y + 1) + x + 1;
}
std::pair<int, int> Board::idx_to_coords(uint16_t idx)
{
#if DEBUG
    assert(idx >= 0 && (unsigned int)idx < board.size());
#endif
    return std::pair<int, int>(idx / (BOARD_SIZE + 2) - 1, idx % (BOARD_SIZE + 2) - 1);
}

void Board::check_position(uint16_t idx)
{
    assert(chain_roots[idx] != 0);
    assert(chain_liberties[chain_roots[idx]] != 0);
    assert(chain_sizes[chain_roots[idx]] != 0);
}

pointType Board::get_point(uint16_t idx)
{
    return board[idx];
}

void Board::set_point(uint16_t idx, pointType value)
{
    pointType current_state = board[idx];
#if DEBUG
    assert(value != pointType::BLANK);
    assert(current_state != value);
    assert(!(current_state == BLACK && value == WHITE));
    assert(!(current_state == WHITE && value == BLACK));
#endif

    // xor to erase stone
    zobrist ^= (zobrist_hashes_black[idx]) * (current_state == pointType::BLACK && value == pointType::EMPTY);

    zobrist ^= (zobrist_hashes_white[idx]) * (current_state == pointType::WHITE && value == pointType::EMPTY);

    // xor to add stone
    zobrist ^= (zobrist_hashes_black[idx]) * (value == pointType::BLACK);
    zobrist ^= (zobrist_hashes_white[idx]) * (value == pointType::WHITE);

    board[idx] = value;

    switch (current_state)
    {
    case pointType::EMPTY:
        black_count += (value == pointType::BLACK);
        white_count += (value == pointType::WHITE);
        empty_count--;
        break;
    case pointType::BLACK:
        black_count--;
        empty_count++;
        break;
    case pointType::WHITE:
        white_count--;
        empty_count++;
        break;
    case BLANK:
        assert(false);
        break;

        //     default:
        // #if DEBUG
        //         assert(false);
        // #endif
    }

    if (value == pointType::EMPTY)
    {
        std::array<int, 4> prev_chains{};
        uint16_t prev_chain_count = 0;
        for (uint16_t i = 0; i < 4; i++)
        {
            uint16_t neighbor = idx + directions[i];
            if (chain_roots[neighbor] != 0)
            {
                uint16_t chain_id = chain_roots[neighbor];

                bool dupl = false;
                for (uint16_t x = 0; x < prev_chain_count; x++)

                {
                    if (prev_chains[x] == chain_id)
                    {
                        dupl = true;
                        break;
                    }
                }

                if (!dupl)
                {
                    prev_chains[prev_chain_count] = chain_id;
                    prev_chain_count++;
                    chain_liberties[chain_id]++;
                }
            }
        }
    }
}

bool Board::check_play(uint16_t idx)
{
    bool color_to_move = whose_turn();

    struct nbrs neighbors = get_nbrs(idx);

    if (board[idx] != pointType::EMPTY)
    {
        return false;
    }

    if (is_suicide(idx))
    {
        return false;
    }

    // ko checking

    Board copy = *this;
    copy.update_chains(idx);

    if (color_to_move)
    {
        // black to move
        copy.set_point(idx, pointType::BLACK);
        return black_ko_hash != copy.get_hash();
        // if hashes are different not a repeat (except in case of hash collision ig)
    }
    else
    {
        // white to move
        copy.set_point(idx, pointType::WHITE);
        return white_ko_hash != copy.get_hash();
        // if hashes are different not a repeat (except in case of hash collision ig)
    }
}

bool Board::is_suicide(uint16_t idx)
{
    // check if move is suicide
    // if all neighboring opposite color chains have at least 2 liberties (one for stone to be added and another one for safety, they cant be captured)
    //
    // and no neighboring same color chain has at least 2 liberties (one for stone to be added, it can be captured) then it is suicide

    // if a neighboring opposite color chain has < 2 liberties or a neighboring same color chain has > 1 liberties then it is not suicide
    bool color_to_move = whose_turn();

    for (uint16_t i = 0; i < 4; i++)
    {
        uint16_t chain_id = chain_roots[idx + directions[i]];
        switch (board[idx + directions[i]])
        {
        case pointType::BLANK:
            break;
        case pointType::EMPTY:
            // if there's a liberty its not suicide
            return false;
        case pointType::BLACK:
            if (color_to_move)
            {
                // same color chain
                if (chain_liberties[chain_id] > 1)
                {
                    return false;
                }
            }
            else
            // diff color chain
            {
                if (chain_liberties[chain_id] < 2)
                {
                    return false;
                }
            }
            break;
        case pointType::WHITE:
            if (!color_to_move)
            {
                // same color chain
                if (chain_liberties[chain_id] > 1)
                {
                    return false;
                }
            }
            else
            // diff color chain
            {
                if (chain_liberties[chain_id] < 2)
                {
                    return false;
                }
            }
            break;
        }
    }

    return true;
}

void Board::create_chain(uint16_t idx)
{
    chain_roots[idx] = idx;
    chain_liberties[idx] = get_liberties(idx);
    chain_sizes[idx] = 1;
}

void Board::extend_chain(uint16_t idx, uint16_t adj_stone)
{
    uint16_t chain_id = chain_roots[adj_stone];
    chain_roots[idx] = chain_id;
    chain_sizes[chain_id]++;
    chain_liberties[chain_id]--;

    for (uint16_t i = 0; i < 4; i++)
    {
        if (board[idx + directions[i]] == pointType::EMPTY)
        // for each liberty of the new stone
        {
            bool already_counted = false;
            for (uint16_t j = 0; j < 4; j++)
            {
                if (directions[i] + directions[j] == 0)
                {
                    continue;
                }
                if (chain_roots[idx + directions[i] + directions[j]] == chain_id)
                {
                    // check that it's not already counted as a chain liberty
                    already_counted = true;
                    break;
                }
            }
            if (!already_counted)
            {
                chain_liberties[chain_id]++;
                // if a liberty of the new stone doesn't border the chain already then add it for a new liberty
            }
        }
    }
}

void Board::merge_chains(std::array<uint16_t, 4> neighbor_roots, uint16_t num_neighbors, uint16_t idx)
{
#if DEBUG
    assert(num_neighbors > 1);
    for (uint16_t i = 0; i < num_neighbors; i++)
    {
        if (i < num_neighbors)
        {
            assert(neighbor_roots[i] != 0);
        }
        else
        {
            assert(neighbor_roots[i] == 0);
        }
    }
#endif

    uint16_t new_root = neighbor_roots[0];
    // pick root of first chain to be root of new merged one

    uint16_t chain_size = chain_sizes[new_root] + 1;
    // add 1 for new stone

    for (uint16_t i = 1; i < num_neighbors; i++)
    {
        chain_size += chain_sizes[neighbor_roots[i]];
        chain_sizes[neighbor_roots[i]] = 0;
        // sum collective size and clear out afterwards
    }

    for (uint16_t i = 0; i < num_neighbors; i++)
    {
        chain_liberties[neighbor_roots[i]] = 0;
        // update sizes and liberties
    }
    chain_sizes[new_root] = chain_size;

    // update chain roots

    for (uint16_t i = 0; i < NUM_POINTS; i++)
    {
        if (chain_roots[i] == 0)
        {
            // if empty
            continue;
        }
        for (uint16_t j = 1; j < num_neighbors; j++)
        {

            if (chain_roots[i] == neighbor_roots[j])
            {
                chain_roots[i] = new_root;
                break;
            }
        }
    }

    chain_roots[idx] = new_root;

    uint16_t num_liberties = 0;
    for (uint16_t i = 0; i < NUM_POINTS; i++)
    {
        if (board[i] == pointType::EMPTY)
        {
            if (i == idx)
            {
                continue;
            }
            for (uint16_t j = 0; j < 4; j++)
            {
                if (chain_roots[i + directions[j]] == new_root)
                {
                    num_liberties++;
                    break;
                }
            }
        }
    }

    chain_liberties[new_root] = num_liberties;
}

void Board::capture_chain(uint16_t chain_root)
{
#if DEBUG
    assert(chain_liberties[chain_root] == 0);
#endif
    chain_sizes[chain_root] = 0;

    for (uint16_t i = 0; i < NUM_POINTS; i++)
    {
        if (chain_roots[i] == chain_root)
        {
            chain_roots[i] = 0;
            set_point(i, pointType::EMPTY);
        }
    }
    chain_liberties[chain_root] = 0;
    // we need this since set point adds liberties to adjacent chains on removal
}

uint16_t Board::get_liberties(uint16_t idx)
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

void Board::print_board()
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
}

nbrs Board::get_nbrs(uint16_t idx)
{
    nbrs n;
    n.edges = 0;
    n.liberties = 0;
    n.black = 0;
    n.white = 0;
    uint8_t num_edges = 0;
    uint8_t num_libs = 0;
    uint8_t num_black = 0;
    uint8_t num_white = 0;

    for (uint8_t i = 0; i < 4; i++)
    {
        switch (board[idx + directions[i]])
        {
        case pointType::BLANK:
            num_edges++;
            n.edges |= uint8_t(1 << i);
            break;
        case pointType::EMPTY:
            num_libs++;
            n.liberties |= uint8_t(1 << i);
            break;
        case pointType::BLACK:
            num_black++;
            n.black |= uint8_t(1 << i);
            break;
        case pointType::WHITE:
            num_white++;
            n.white |= uint8_t(1 << i);
            break;
        }
    }

#if DEBUG
    assert((num_edges + num_libs + num_black + num_white) == 4);
#endif

    n.edges |= uint8_t(num_edges << 4);
    n.liberties |= uint8_t(num_libs << 4);
    n.black |= uint8_t(num_black << 4);
    n.white |= uint8_t(num_white << 4);
    return n;
}

uint64_t Board::get_hash()
{
    return zobrist;
}

bool Board::make_play(uint16_t x, uint16_t y)
{
    bool color_to_move = whose_turn();
    uint16_t idx = coords_to_idx(x, y);

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
        print_board();
        check_for_errors();
#endif
        return true;
    }
    return false;
}

bool Board::whose_turn()
{
    return (play_count & 1) == 0;
}

uint16_t Board::get_play_count()
{
    return play_count;
}

void Board::check_for_errors()
{
    std::array<uint16_t, NUM_POINTS> liberties_check{};
    std::array<uint16_t, NUM_POINTS> sizes_check{};
    for (uint16_t i = 0; i < NUM_POINTS; i++)
    {
        switch (board[i])
        {
        case pointType::BLACK:
        case pointType::WHITE:
            sizes_check[chain_roots[i]]++;

            assert(chain_roots[i] != 0);
            if (chain_liberties[chain_roots[i]] == 0)
            {
                std::cout << "no libs " << i << " " << chain_roots[i] << " " << chain_liberties[chain_roots[i]] << '\n';
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
            std::cout << "libs " << i << " " << liberties_check[i] << " " << chain_liberties[i] << '\n';
            assert(false);
        }
        if (sizes_check[i] != chain_sizes[i])
        {
            std::cout << "size " << i << " " << sizes_check[i] << " " << chain_sizes[i] << '\n';

            assert(false);
        }
    }
}

void Board::update_chains(uint16_t idx)
{
    struct nbrs n = get_nbrs(idx);

    bool side = whose_turn();

    pointType oppositeSide = side ? pointType::WHITE : pointType::BLACK;
    pointType sameSide = side ? pointType::BLACK : pointType::WHITE;

    uint16_t num_same_color = ((side ? n.black : n.white) & COUNT) >> 4;

    std::array<int, 4> prev_chains{};
    uint16_t prev_chain_count = 0;
    for (uint16_t i = 0; i < 4; i++)
    {
        uint16_t neighbor = idx + directions[i];
        if (board[neighbor] == oppositeSide)
        {
            uint16_t chain_id = chain_roots[neighbor];
#if DEBUG
            assert(chain_roots[neighbor] != 0);
            assert(chain_liberties[chain_roots[neighbor]] != 0);
            assert(chain_sizes[chain_roots[neighbor]] != 0);
#endif
            bool dupl = false;
            for (uint16_t x = 0; x < prev_chain_count; x++)

            {
                if (prev_chains[x] == chain_id)
                {
                    dupl = true;
                    break;
                }
            }

            if (!dupl)
            {
                prev_chains[prev_chain_count] = chain_id;
                prev_chain_count++;

                chain_liberties[chain_id]--;
                if (chain_liberties[chain_id] == 0)
                {
                    capture_chain(chain_id);
                }
            }
        }
    }

    if (num_same_color == 0)
    {
        // new chain
        create_chain(idx);
    }

    else if (num_same_color == 1)
    {
        // extension
        for (uint16_t i = 0; i < 4; i++)
        {
            uint16_t neighbor = idx + directions[i];
            if (board[neighbor] == sameSide)
            {
                extend_chain(idx, neighbor);
                break;
            }
        }
    }

    else
    {
        // merger or extension

        std::array<uint16_t, 4> same_color_chains{};
        uint16_t same_color_count = 0;
        for (uint16_t i = 0; i < 4; i++)
        {
            uint16_t neighbor = idx + directions[i];
            if (board[neighbor] == sameSide)
            {
                uint16_t chain_id = chain_roots[neighbor];

                bool dupl = false;
                for (uint16_t x = 0; x < same_color_count; x++)
                {
                    if (same_color_chains[x] == chain_id)
                    {
                        dupl = true;
                        break;
                    }
                }

                if (!dupl)
                {
                    same_color_chains[same_color_count] = chain_id;
                    same_color_count++;
                }
            }
        }

#if DEBUG
        assert(same_color_count > 0);
#endif

        if (same_color_count == 1)
        {
            extend_chain(idx, same_color_chains[0]);
        }
        else
        {
            merge_chains(same_color_chains, same_color_count, idx);
        }
    }
}