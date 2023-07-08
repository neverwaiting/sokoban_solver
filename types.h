#ifndef WSUN_SOKOBAN_TYPES_H
#define WSUN_SOKOBAN_TYPES_H

#include <inttypes.h>
#include <vector>

enum SquareType : unsigned {
  kFloor = 0,
  kWall = 1 << 0,
  kBox = 1 << 1,
  kGoal = 1 << 2,
  kPlayer = 1 << 3,
  kBoxOnGoal = kGoal | kBox,
  kPlayerOnGoal = kGoal | kPlayer,
  kEmpty = 1 << 4
};

using Map = std::vector<SquareType>;

struct Level {
  int file;
  int rank;
  Map map;
};

using LevelArray = std::vector<Level>;

using Direction = int;

#endif
