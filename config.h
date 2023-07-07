#ifndef WSUN_SOKOBAN_CONFIG_H
#define WSUN_SOKOBAN_CONFIG_H

#include "types.h"
#include <map>
#include <string>

static const std::map<char, SquareType> kSymbolsMap = {
  { ' ', SquareType::kFloor },
  { '#', SquareType::kWall },
  { '$', SquareType::kBox },
  { '.', SquareType::kGoal },
  { '@', SquareType::kPlayer },
  { '*', SquareType::kBoxOnGoal },
  { '+', SquareType::kPlayerOnGoal },
};

static const char* kLevelDataDirPath = "screens";

#endif
