#ifndef WSUN_SOKOBAN_SOLVER_H
#define WSUN_SOKOBAN_SOLVER_H

#include "parser.h"
#include "zobrist.h"
#include "deadlock.h"
#include <queue>
#include <memory>
#include <cassert>
#include <algorithm>
#include <climits>

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
  Zobrist zobrist;
  std::vector<Zobrist> playerZobrists;
  std::vector<Zobrist> boxZobrists;

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

    playerZobrists.resize(map.size());
    boxZobrists.resize(map.size());
    RC4 rc4;
    std::for_each(playerZobrists.begin(), playerZobrists.end(), [&rc4](Zobrist& zobrist) {
        zobrist = Zobrist(rc4);
    });
    std::for_each(boxZobrists.begin(), boxZobrists.end(), [&rc4](Zobrist& zobrist) {
        zobrist = Zobrist(rc4);
    });
    zobrist.XOR(playerZobrists[playerSolt]);
    for (int box : boxs) {
      zobrist.XOR(boxZobrists[box]);
    }
  }

  Board(const Board& rhs) {
    this->goals = std::set<int>(rhs.goals.begin(), rhs.goals.end());
    this->boxs = std::set<int>(rhs.boxs.begin(), rhs.boxs.end());
    this->playerSolt = rhs.playerSolt;
    this->file = rhs.file;
    this->map = rhs.map;
  }

  bool compareBoard(const Board& rhs) const {
    return std::equal(boxs.begin(), boxs.end(), rhs.boxs.begin()) && playerSolt == rhs.playerSolt;
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

  void print(const DeadLock& dl) const {
    int j = 0;
    for (int i = 0; i < map.size(); ++i) {
      SquareType st = map[i];
      if (dl.isDeadSolt(i)) {
        std::cout << "X";
      } else {
        std::cout << kSymbolsReverseMap.find(st)->second;
      }
      if (j == file - 1) std::cout << std::endl;
      j = j == file - 1 ? 0 : j + 1;
    }
    std::cout << std::endl;
  }

  void print(const std::vector<int>& boxReach) const {
    int j = 0;
    for (int i = 0; i < map.size(); ++i) {
      SquareType st = map[i];
      if (boxReach[i] != INT_MAX) {
        std::cout << "O";
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

static void calcReachableTiles(const Board& board, Reach& reach) {
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

static void getPushes(const Board& board, std::vector<Push>& pushes) {
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
  // board.print(reach);
}

static double distance(int start, int end, int file) {
  int startx = start / file;
  int starty = start % file;
  int endx = end / file;
  int endy = end % file;
  return std::abs(startx - endx) + std::abs(starty - endy);
}

static double costForManhattan(const Board& board) {
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

static void doPush(Board& board, const Push& push) {
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

  board.zobrist.XOR(board.playerZobrists[push.playerSolt]);
  board.zobrist.XOR(board.playerZobrists[movePlayerSolt]);
  board.zobrist.XOR(board.boxZobrists[push.boxSolt]);
  board.zobrist.XOR(board.boxZobrists[moveBoxSolt]);
  // std::cout << "do push after: " << board.playerSolt << std::endl;
}

static void undoPush(Board& board, const Push& push) {
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

  board.zobrist.XOR(board.playerZobrists[push.playerSolt]);
  board.zobrist.XOR(board.playerZobrists[movePlayerSolt]);
  board.zobrist.XOR(board.boxZobrists[push.boxSolt]);
  board.zobrist.XOR(board.boxZobrists[moveBoxSolt]);
  // std::cout << "undo push after: " << board.playerSolt << std::endl;
}

static bool checkGameOver(const Board& board) {
  std::set<int> temp(board.goals);
  temp.insert(board.boxs.begin(), board.boxs.end());
  return temp.size() == board.goals.size();
}

static void astarSearch(Board& board) {
  DeadLock dl;
  dl.generate(board);
  int generateNodes = 0;
  int explorNodes = 0;

  std::vector<Push> pushes;
  getPushes(board, pushes);
  printf("generate pushes: %lu\n", pushes.size());
  if (pushes.empty()) return;

  // std::set<Zobrist> visitedZobrist;
  // visitedZobrist.insert(board.zobrist);
  std::vector<Board> visitedBoard;
  visitedBoard.push_back(board);
  // printf("before people pos: %d\n", board.playerSolt);
  PriorityQueue<PushPtr, double> frontier;
  // std::queue<PushPtr> frontier;
  for (const auto& push : pushes) {
    doPush(board, push);
    frontier.push(PushPtr(new Push(push)), costForManhattan(board));
    // frontier.push(PushPtr(new Push(push)));
    printf("push item: %d, %d\n", push.boxSolt, push.dir);
    undoPush(board, push);
  }
  // printf("after people pos: %d\n", board.playerSolt);

  while (!frontier.empty()) {
    PushPtr push = frontier.pop();
    // PushPtr push = frontier.front();
    // frontier.pop();

    board = push->board;
    ++explorNodes;
    if (explorNodes >= 100000) break;
    // printf("explor nodes: %d\n", explorNodes);
    // printf("current people pos: %d, %d\n", board.playerSolt, push->boxSolt);

    // printf("do push: %d, %d\n", push->boxSolt, push->dir);
    doPush(board, *push);
    // board.print();
    // if (visitedZobrist.find(board.zobrist) != visitedZobrist.end()) {
    //   std::cout << "repeat situation" << std::endl;
    //   continue;
    // }
    // visitedZobrist.insert(board.zobrist);
    if (std::find(visitedBoard.begin(), visitedBoard.end(), board) != visitedBoard.end()) {
      // std::cout << "repeat situation" << std::endl;
      continue;
    }
    visitedBoard.push_back(board);
    // printf("current people pos: %d, %d\n", board.playerSolt, push->boxSolt);
    if (checkGameOver(board)) {
      printf("explor nodes: %d\n", explorNodes);
      printf("find best way in Astar search, generateNodes: %d, explorNodes: %d\n", generateNodes, explorNodes);
      break;
    }

    std::vector<Push> pushes;
    getPushes(board, pushes);
    // printf("generate pushes: %lu\n", pushes.size());
    for (auto p : pushes) {
      bool isdead = dl.isDeadSolt(p.boxSolt + p.dir);
      if (isdead) continue;
      PushPtr pushPtr(new Push(p));
      // pushPtr->board.print();
      // frontier.push(pushPtr);
      // printf("push item: %d, %d, %d\n", p.boxSolt, p.dir, 0);
      doPush(board, p);
      int cost = costForManhattan(board);
      undoPush(board, p);
      frontier.push(pushPtr, cost);
    }
  }
}

#endif
