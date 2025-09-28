#include <cstdint>
#ifndef CONFIG_H
#define CONFIG_H
#define DEBUG false
#define PROFILE false

static constexpr auto BOARD_SIZE = 9;

static constexpr auto komi = 7.5f;

static constexpr auto NUM_POINTS = (BOARD_SIZE + 2) * (BOARD_SIZE + 2);
static constexpr uint16_t PASS = 0;   // on board edge
static constexpr uint16_t RESIGN = 1; // on board edge
#endif