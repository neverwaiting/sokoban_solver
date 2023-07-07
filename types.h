#ifndef WSUN_SOKOBAN_TYPES_H
#define WSUN_SOKOBAN_TYPES_H

#include <inttypes.h>
#include <vector>

enum SquareType : uint32_t {
  kFloor = 1 << 0,
  kWall = 1 << 1,
  kBox = 1 << 2,
  kGoal = 1 << 3,
  kPlayer = 1 << 4,
  kBoxOnGoal = kGoal | kBox,
  kPlayerOnGoal = kGoal | kPlayer
};

struct Map {
  int file;
  int rank;
  std::vector<SquareType> map;
};

#endif
