#ifndef WSUN_SOKOBAN_PARSER_H
#define WSUN_SOKOBAN_PARSER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <set>
#include <string>
#include "config.h"


static SquareType& operator |=(SquareType& left, SquareType right) {
  left = static_cast<SquareType>(static_cast<unsigned>(left) | static_cast<unsigned>(right));
  return left;
}

static SquareType& operator ^=(SquareType& left, SquareType right) {
  left = static_cast<SquareType>(static_cast<unsigned>(left) ^ static_cast<unsigned>(right));
  return left;
}

struct LevelSorter {
  bool operator()(const std::string& left, const std::string& right) const {
    auto l = left.substr(left.rfind('.') + 1);
    auto r = right.substr(right.rfind('.') + 1);
    return l.length() == r.length() ? l < r : l.length() < r.length();
  }
};

static void getAllFileInPath(const char* path, std::set<std::string, LevelSorter>& files) {
  DIR* pDir = opendir(path);
  if (pDir == nullptr) {
    std::cout << "path: " << path << " is not exist!" << std::endl;
    return;
  }
  struct dirent* dire = nullptr;
  while ((dire= readdir(pDir)) != nullptr) {
    if (dire->d_type == DT_REG) {
      files.insert(std::string(path) + '/' + dire->d_name);
    }
  }
  closedir(pDir);
}

static void getMap(const std::string& levelStr, Level& level) {
  level.file = 0;
  int file = 0;
  int rank = 0;
  for (char c : levelStr) {
    if (c != '\n') {
      ++file;
      level.file = std::max(level.file, file);
    } else {
      ++rank;
      file = 0;
    }
  }
  level.rank = rank;
  level.map.resize(level.rank * level.file, SquareType::kFloor);
  int i = 0;
  int j = 0;
  for (char c : levelStr) {
    if (c == '\n') {
      j = 0;
      ++i;
    }
    else {
      int idx = i * level.file + j;
      SquareType type = kSymbolsMap.find(c)->second;
      if (type & SquareType::kWall) level.map[idx] |= SquareType::kWall;
      if (type & SquareType::kBox) level.map[idx] |= SquareType::kBox;
      if (type & SquareType::kGoal) level.map[idx] |= SquareType::kGoal;
      if (type & SquareType::kPlayer) level.map[idx] |= SquareType::kPlayer;
      ++j;
    }
  }
}

static void getAllLevels(LevelArray& levels) {
  std::set<std::string, LevelSorter> files;
  getAllFileInPath(kLevelDataDirPath, files); 
  for (auto file : files) {
    std::ostringstream oss;
    oss << std::ifstream(file).rdbuf();
    Level level;
    getMap(oss.str(), level);
    levels.push_back(level);
  }
}

static void printLevel(const Level& level) {
  int j = 0;
  for (SquareType st : level.map) {
    std::cout << kSymbolsReverseMap.find(st)->second;
    if (j == level.file - 1) std::cout << std::endl;
    j = j == level.file - 1 ? 0 : j + 1;
  }
  std::cout << std::endl;
}

#endif
