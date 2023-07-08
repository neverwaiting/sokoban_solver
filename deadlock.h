#ifndef WSUN_SOKOBAN_DEADLOCK_H_
#define WSUN_SOKOBAN_DEADLOCK_H_

#include "types.h"
#include "solver.h"
#include <unordered_map>
#include <unordered_set>

namespace sokoban {

class DeadLock {
 public:
  // DeadLock();
  // ~DeadLock();

  void generate(const Board& board);
  bool isDeadSolt(int pos) const;
  const std::unordered_map<int, std::vector<int>>& distanceGoals() const;
  const std::unordered_set<int>& deadBlocks() const;

 private:
  std::unordered_map<int, std::vector<int>> distanceGoals_;
  std::unordered_set<int> deadblocks_;
};

} // namespace sokoban

#endif // #ifndef DEADLOCK_H
