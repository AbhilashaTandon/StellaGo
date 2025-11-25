/* Looks through all the SGF files and finds the average move number where each square of the board is occupied. The starpoints will be first */
#include "SGFReader.h"

void search_files()
{
    std::array<std::pair<int, int>, 441> move_count_sums;
    for (const auto &sgf_file : std::filesystem::recursive_directory_iterator(path_to_games))
    {
        if (sgf_file.path().extension() == ".sgf")
        {
            std::ifstream game_file(sgf_file.path(), std::ios::binary);
            if (!game_file.is_open())
            {
                std::cout << "Failed to open file " << sgf_file.path() << '\n';
                return;
            }
            else
            {
                // std::cout << sgf_file.path() << std::endl;
                SGFReader file = SGFReader(sgf_file.path());
                file.record_moves(move_count_sums);
                // break;
            }
        }
    }

    for (int i = 0; i < 19; i++)
    {
        for (int j = 0; j < 19; j++)
        {
            int idx = (i + 1) * 21 + j + 1;
            std::pair<int, int> record = move_count_sums[idx];
            std::cout << double(record.first) / double(record.second) << " \t ";
        }
        std::cout << '\n';
    }
}

void SGFReader::record_moves(std::array<std::pair<int, int>, 441> &move_count_sums)
{

    std::ifstream ifs;
    std::string line;
    ifs.open(file_path, std::ifstream::in);
    while (std::getline(ifs, line))
    {
        if (line.length() && line.at(0) == ';')
        {
            break;
        }
    }

    int move_count = 0;

    do
    {
        std::string move = "";
        for (uint16_t i = 1; i < line.length(); i++)
        {
            if (line[i] != ';')
            {
                move.push_back(line[i]);
                continue;
            }

            if (move.length() > 3)
            {
                int x_coord = move.at(2) - 'a';
                int y_coord = move.at(3) - 'a';
                int idx = (x_coord + 1) * 21 + y_coord + 1;
                if (idx < 0 || idx >= 441)
                {
                    continue;
                }
                std::pair<int, int> current_count = move_count_sums[idx];
                move_count_sums[idx] = std::pair<int, int>(current_count.first + move_count, current_count.second + 1);
                // moves.push_back(std::pair<uint8_t, uint8_t>(x_coord, y_coord));
                move_count++;
                move = "";
            }
        }
    } while (std::getline(ifs, line));
}

SGFReader::SGFReader(std::string file_path) : file_path(file_path)
{
}
