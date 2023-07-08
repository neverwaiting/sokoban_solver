#ifndef WSUN_SOKOBAN_CONFIG_H
#define WSUN_SOKOBAN_CONFIG_H

#include "types.h"
#include <map>
#include <string>
#include <array>

static const std::map<char, SquareType> kSymbolsMap = {
  { ' ', SquareType::kFloor },
  { '#', SquareType::kWall },
  { '$', SquareType::kBox },
  { '.', SquareType::kGoal },
  { '@', SquareType::kPlayer },
  { '*', SquareType::kBoxOnGoal },
  { '+', SquareType::kPlayerOnGoal },
};

static const std::map<SquareType, char> kSymbolsReverseMap = {
  { SquareType::kFloor, ' ' },
  { SquareType::kWall, '#' },
  { SquareType::kBox, '$' },
  { SquareType::kGoal, '.' },
  { SquareType::kPlayer, '@' },
  { SquareType::kBoxOnGoal, '*' },
  { SquareType::kPlayerOnGoal, '+' },
};

// left right up down
static const Direction Left = 0;
static const Direction Right = 1;
static const Direction Up = 2;
static const Direction Down = 3;
static std::array<Direction, 4> kDirection = { -1, 1, 0, 0 };

static const char* kLevelDataDirPath = "screens";

#endif
