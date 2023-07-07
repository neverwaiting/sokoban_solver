#include <iostream>
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <set>
#include <string>
#include "config.h"

struct LevelSorter {
  bool operator()(const std::string& left, const std::string& right) const {
    auto l = left.substr(left.rfind('.') + 1);
    auto r = right.substr(right.rfind('.') + 1);
    return l.length() == r.length() ? l < r : l.length() < r.length();
  }
};

void getAllFileInPath(const char* path, std::set<std::string, LevelSorter>& files) {
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

void getMap(const std::string& mapstr, Map& map) {
  map.file = 0;
  int file = 0;
  int rank = 0;
  for (char c : mapstr) {
    if (c != '\n') {
      ++file;
      map.file = std::max(map.file, file);
    } else {
      ++rank;
      file = 0;
    }
  }
  map.rank = rank;
  map.map.resize(map.rank * map.file, SquareType::kFloor);
  std::cout << map.file << ", "  << map.rank << ", " << map.map.size() << std::endl;
  int i = 0;
  int j = 0;
  for (char c : mapstr) {
    if (c == '\n') {
      j = 0;
      ++i;
    }
    else {
      int idx = i * map.file + j;
      SquareType type = kSymbolsMap.find(c)->second;
      if (type & SquareType::kWall) map.map[idx] |= SquareType::kWall;
      if (type & SquareType::kBox) map.map[idx] |= SquareType::kBox;
      if (type & SquareType::kGoal) map.map[idx] |= SquareType::kGoal;
      if (type & SquareType::kPlayer) map.map[idx] |= SquareType::kPlayer;
      ++j;
    }
  }
}

int main(int argc, char** argv) {
  std::set<std::string, LevelSorter> files;
  getAllFileInPath(kLevelDataDirPath, files); 
  std::vector<Map> levels;
  for (auto file : files) {
    std::ostringstream oss;
    oss << std::ifstream(file).rdbuf();
    std::cout << oss.str() << std::endl;
    Map level;
    getMap(oss.str(), level);
    levels.push_back(level);
  }

  return 0;
}
