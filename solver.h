#ifndef WSUN_SOKOBAN_SOLVER_H
#define WSUN_SOKOBAN_SOLVER_H

#include "parser.h"
#include <queue>
#include <memory>
#include <cassert>

namespace {

template <class Item, class PriorityValue>
struct PriorityQueue {
  typedef std::pair<PriorityValue, Item> Element;
  struct CompareFn {
    bool operator()(const Element& l, const Element& r) {
      return l.first > r.first;
    }
  };
  typedef std::priority_queue<Element, std::vector<Element>,CompareFn> InnerQueue;

  inline bool empty() const {
    return queue_.empty();
  }

  inline void push(const Item& item, PriorityValue pri) {
    queue_.emplace(pri, item);
  }

  Item pop() {
    Item item = queue_.top().second;
    queue_.pop();
    return item;
  }

  InnerQueue queue_;
};

} // namespace 

enum ReachState{
  kUnvisited,
  kReachable,
  kReachableBox
};

struct Reach {
  int minReachableSolt;
  std::vector<ReachState> tiles;

  bool isReachableBox(int solt) const {
    return tiles[solt] == ReachState::kReachableBox;
  }

  bool isReachable(int solt) const {
    return tiles[solt] == ReachState::kReachable;
  }
};

struct Board {
  std::set<int> goals;
  std::set<int> boxs;
  int playerSolt;
  int file;
  Map map;

  Board(const Level& level) {
    for (int i = 0; i < level.map.size(); ++i) {
      SquareType st = level.map[i];
      if (st & SquareType::kGoal) goals.insert(i);
      if (st & SquareType::kBox) boxs.insert(i);
      if (st & SquareType::kPlayer) playerSolt = i;
    }
    file = level.file;
    map = level.map;
    kDirection[Up] = -level.file;
    kDirection[Down] = level.file;
  }

  Board(const Board& rhs) {
    this->goals = std::set<int>(rhs.goals.begin(), rhs.goals.end());
    this->boxs = std::set<int>(rhs.boxs.begin(), rhs.boxs.end());
    this->playerSolt = rhs.playerSolt;
    this->file = rhs.file;
    this->map = rhs.map;
    assert(*this == rhs);
  }

  bool operator==(const Board& rhs) const {
    if (goals.size() != rhs.goals.size()) return false;
    for (auto it = goals.begin(), itr = rhs.goals.begin();
         it != goals.end(); ++it, ++itr) {
      if (*it != *itr) return false;
    }
    if (boxs.size() != rhs.boxs.size()) return false;
    for (auto it = boxs.begin(), itr = rhs.boxs.begin();
         it != boxs.end(); ++it, ++itr) {
      if (*it != *itr) return false;
    }
    if (map.size() != rhs.map.size()) return false;
    for (auto it = map.begin(), itr = rhs.map.begin();
         it != map.end(); ++it, ++itr) {
      if (*it != *itr) return false;
    }
    return playerSolt == rhs.playerSolt && file == rhs.file;
  }

  bool operator!=(const Board& rhs) const {
    return !operator==(rhs);
  }

  bool isNeighborWithBox(int solt) const {
    for (auto dir : kDirection) {
      int dest = solt + dir;
      if (map[dest] & SquareType::kBox) return true;
    }
    return false;
  }

  void print(const Reach& reach) const {
    int j = 0;
    for (int i = 0; i < map.size(); ++i) {
      SquareType st = map[i];
      if (reach.isReachable(i) && !(st & SquareType::kPlayer)) {
        std::cout << "R";
      } else if (reach.isReachableBox(i)) {
        std::cout << "B";
      } else {
        std::cout << kSymbolsReverseMap.find(st)->second;
      }
      if (j == file - 1) std::cout << std::endl;
      j = j == file - 1 ? 0 : j + 1;
    }
    std::cout << std::endl;
  }

  void print() const {
    int j = 0;
    for (int i = 0; i < map.size(); ++i) {
      SquareType st = map[i];
      std::cout << kSymbolsReverseMap.find(st)->second;
      if (j == file - 1) std::cout << std::endl;
      j = j == file - 1 ? 0 : j + 1;
    }
    std::cout << std::endl;
  }

};

struct Push;
using PushPtr = std::shared_ptr<Push>;

struct Push {
  int boxSolt;
  Direction dir;
  int playerSolt;

  int children;
  PushPtr parent;
  Board board;

  Push(int boxSolt, Direction dir, int playerSolt, const Board& board)
    : boxSolt(boxSolt),
      dir(dir),
      playerSolt(playerSolt),
      children(0),
      parent(nullptr),
      board(board)
  {
  }


  Push(const Push&) = default;

  void removeFromParent(Board& board);
  void addToParent(const PushPtr& parent);
};

void calcReachableTiles(const Board& board, Reach& reach) {
  reach.minReachableSolt = board.playerSolt;
  reach.tiles.resize(board.map.size(), ReachState::kUnvisited);

  std::queue<int> q;
  q.push(reach.minReachableSolt);
  while (!q.empty()) {
    int solt = q.front();
    int st = board.map[solt];
    if (!(st & SquareType::kWall) &&
        reach.tiles[solt] == ReachState::kUnvisited) {

      reach.tiles[solt] = board.isNeighborWithBox(solt) ?
        ReachState::kReachableBox : ReachState::kReachable;
      for (auto dir : kDirection) {
        int dest = solt + dir;
        if (!(board.map[dest] & kBox)) q.push(dest);
      }
      reach.minReachableSolt = std::min(solt, reach.minReachableSolt);
    }
    q.pop();
  }
}

void getPushes(const Board& board, std::vector<Push>& pushes) {
  Reach reach;
  calcReachableTiles(board, reach);
  for (int boxSolt : board.boxs) {
    for (auto dir : kDirection) {
      int pushSolt = boxSolt + dir;
      int destSolt = boxSolt - dir;
      if (reach.isReachableBox(pushSolt) &&
          !(board.map[destSolt] & (kWall | kBox))) {
        pushes.emplace_back(boxSolt, -dir, board.playerSolt, board);
      }
    }
  }
  board.print(reach);
}

double distance(int start, int end, int file) {
  int startx = start / file;
  int starty = start % file;
  int endx = end / file;
  int endy = end % file;
  return std::abs(startx - endx) + std::abs(starty - endy);
}

double costForManhattan(const Board& board) {
  const std::set<int>& goals = board.goals;
  const std::set<int>& boxs = board.boxs;

  double d = 0;
  for (auto it_box = boxs.begin(), it_goal = goals.begin();
      it_box != boxs.end();
      ++it_box, ++it_goal)
  {
    d += distance(*it_box, *it_goal, board.file);
  }
  return d;
}

void doPush(Board& board, const Push& push) {
  // std::cout << "do push before: " << board.playerSolt << std::endl;
  int movePlayerSolt = push.boxSolt;
  int moveBoxSolt = push.boxSolt + push.dir;
  board.map[push.playerSolt] ^= SquareType::kPlayer;
  board.map[movePlayerSolt] |= SquareType::kPlayer;
  board.map[push.boxSolt] ^= SquareType::kBox;
  board.map[moveBoxSolt] |= SquareType::kBox;
  board.playerSolt = movePlayerSolt;
  board.boxs.erase(push.boxSolt);
  board.boxs.insert(moveBoxSolt);
  // std::cout << "do push after: " << board.playerSolt << std::endl;
}

void undoPush(Board& board, const Push& push) {
  // std::cout << "undo push before: " << board.playerSolt << std::endl;
  int movePlayerSolt = push.boxSolt;
  int moveBoxSolt = push.boxSolt + push.dir;
  board.map[push.playerSolt] |= SquareType::kPlayer;
  board.map[movePlayerSolt] ^= SquareType::kPlayer;
  board.map[push.boxSolt] |= SquareType::kBox;
  board.map[moveBoxSolt] ^= SquareType::kBox;
  board.playerSolt = movePlayerSolt;
  board.boxs.insert(push.boxSolt);
  board.boxs.erase(moveBoxSolt);
  // std::cout << "undo push after: " << board.playerSolt << std::endl;
}

bool checkGameOver(const Board& board) {
  std::set<int> temp(board.goals);
  temp.insert(board.boxs.begin(), board.boxs.end());
  return temp.size() == board.goals.size();
}

void astarSearch(Board& board) {
  int generateNodes = 0;
  int explorNodes = 0;

  std::vector<Push> pushes;
  getPushes(board, pushes);
  printf("generate pushes: %lu\n", pushes.size());
  if (pushes.empty()) return;

  // printf("before people pos: %d\n", board.playerSolt);
  PriorityQueue<PushPtr, double> frontier;
  for (const auto& push : pushes) {
    doPush(board, push);
    frontier.push(PushPtr(new Push(push)), costForManhattan(board));
    printf("push item: %d, %d\n", push.boxSolt, push.dir);
    undoPush(board, push);
  }
  // printf("after people pos: %d\n", board.playerSolt);

  while (!frontier.empty()) {
    PushPtr push = frontier.pop();
    board = push->board;
    ++explorNodes;
    // printf("current people pos: %d, %d\n", board.playerSolt, push->boxSolt);

    printf("current push: %d, %d\n", push->boxSolt, push->dir);
    doPush(board, *push);
    // printf("current people pos: %d, %d\n", board.playerSolt, push->boxSolt);
    assert(board != push->board);
    if (checkGameOver(board)) {
      printf("find best way in Astar search, generateNodes: %d, explorNodes: %d\n", generateNodes, explorNodes);
      break;
    }

    std::vector<Push> pushes;
    getPushes(board, pushes);
    printf("generate pushes: %lu\n", pushes.size());
    if (pushes.empty()) {
      break;
      continue;
    }
    for (auto p : pushes) {
      doPush(board, p);
      PushPtr pushPtr(new Push(p));
      pushPtr->addToParent(push);
      frontier.push(push, costForManhattan(board));
      printf("push item: %d, %d\n", push->boxSolt, push->dir);
      undoPush(board, p);
    }
    break;
  }
}

void Push::removeFromParent(Board& board) {
  if (--parent->children == 0) {
    parent->removeFromParent(board);
  }
}

void Push::addToParent(const PushPtr& parent) {
  this->parent = parent;
  ++this->parent->children;
}

#endif
