#include "deadlock.h"
#include "solver.h"
#include "config.h"
#include <climits>
#include <queue>
#include <algorithm>

void DeadLock::generate(const Board& m) {
  distanceGoals_.clear();
  kDirection[Up] = -m.file;
  kDirection[Down] = m.file;
  for (auto goal : m.goals) {
    std::vector<int>& path = distanceGoals_[goal];
    path.resize(m.map.size(), INT_MAX);
    path[goal] = 0;

    std::queue<int> frontier;
    frontier.push(goal);
    while (!frontier.empty()) {
      int current = frontier.front();
      frontier.pop();
      for (auto dir : kDirection)
      {
        int boxSolt = current + dir;
        int playerSolt = boxSolt + dir;

        bool visited = path[boxSolt] != INT_MAX;
        if (visited) continue;

        if (!(m.map[boxSolt] & SquareType::kWall) &&
            !(m.map[playerSolt] & SquareType::kWall)) {
          path[boxSolt] = path[current] + 1; 
          frontier.push(boxSolt);
        }
      }
    }
  }

  deadblocks_.clear();
  for (size_t i = 0; i < m.map.size(); ++i)
  {
    if (m.map[i] & (SquareType::kWall | SquareType::kEmpty)) continue;

    auto it = std::find_if(distanceGoals_.begin(), distanceGoals_.end(),
        [i](const std::pair<int, std::vector<int>>& e) {
          return e.second.at(i) != INT_MAX;
        });
    bool unreachable = it == distanceGoals_.end();

    // find dead pos
    if (unreachable) deadblocks_.insert(i);
  }
}

bool DeadLock::isDeadSolt(int pos) const {
  return deadblocks_.find(pos) != deadblocks_.end();
}

const std::unordered_map<int, std::vector<int>>&
DeadLock::distanceGoals() const {
  return distanceGoals_;
}

const std::unordered_set<int>& DeadLock::deadBlocks() const {
  return deadblocks_;
}

