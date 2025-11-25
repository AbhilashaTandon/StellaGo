#ifndef SGF_FILE_H
#define SGF_FILE_H
/* Looks through all the SGF files and finds the average move number where each square of the board is occupied. The starpoints will be first */
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <cstdint>
#include <array>
#include <vector>

class SGFReader
{
public:
    SGFReader(std::string file_path);

    void record_moves(std::array<std::pair<int, int>, 441> &move_count_sums);

protected:
    std::string file_path;
    std::vector<std::pair<uint8_t, uint8_t>> moves;
};

const std::string path_to_games = "games";

void search_files();

#endif