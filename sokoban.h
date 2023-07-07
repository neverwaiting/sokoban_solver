#ifndef WSUN_SOKOBAN_H
#define WSUN_SOKOBAN_H

#include "types.h"
#include <vector>

using Map = std::vector<SquareType>;
struct Sokoban {
  Map map;
};

#endif
