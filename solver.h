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
#include <chrono>

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

struct DynamicData {
  std::set<int> boxes;
  int playerSolt;
  
  bool operator<(const DynamicData& rhs) const {
    return playerSolt == rhs.playerSolt ? boxes < rhs.boxes : playerSolt < rhs.playerSolt;
  }

  bool operator==(const DynamicData& rhs) const {
    return playerSolt == rhs.playerSolt && boxes == rhs.boxes;
  }

  bool operator!=(const DynamicData& rhs) const {
    return !operator==(rhs);
  }

};

struct Board {
  std::set<int> goals;
  std::set<int> boxes;
  int playerSolt;
  int file;
  Map map;
  Zobrist zobrist;
  std::vector<Zobrist> playerZobrists;
  std::vector<Zobrist> boxZobrists;

  void extractDynamicData(DynamicData& data) const {
    data.boxes = boxes;
    data.playerSolt = playerSolt;
  }

  void recoverFromData(const DynamicData& data) {
    for (auto box : boxes) map[box] ^= SquareType::kBox;
    for (auto box : data.boxes) map[box] |= SquareType::kBox;
    map[playerSolt] ^= SquareType::kPlayer;
    map[data.playerSolt] |= SquareType::kPlayer;
    boxes = data.boxes;
    playerSolt = data.playerSolt;
  }

  Board(const Level& level) {
    for (int i = 0; i < level.map.size(); ++i) {
      SquareType st = level.map[i];
      if (st & SquareType::kGoal) goals.insert(i);
      if (st & SquareType::kBox) boxes.insert(i);
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
    for (int box : boxes) {
      zobrist.XOR(boxZobrists[box]);
    }
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

  void printForIcon() const {
    int j = 0;
    for (int i = 0; i < map.size(); ++i) {
      SquareType st = map[i];
      char c = kSymbolsReverseMap.find(st)->second;
      if (c == '$') {
        printf("%s", "📦");
      } else if (c == '*') {
        printf("%s", "🎁");
      } else if (c == '@') {
        printf("%s", "🧍");
      } else if (c == '.') {
        printf("%s", "🏀");
      } else if (c == '#') {
        printf("%s", "🌲");
      } else if (c == '+') {
        printf("%s", "⛹ ");
      } else if (c == ' ') {
        printf("  ");
      }
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

  DynamicData data;

  Push(int boxSolt, Direction dir, int playerSolt, const Board& board)
    : boxSolt(boxSolt),
      dir(dir),
      playerSolt(playerSolt)
  {
    board.extractDynamicData(data);
  }


  Push(const Push&) = default;
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
  for (int boxSolt : board.boxes) {
    for (auto dir : kDirection) {
      int destSolt = boxSolt + dir;
      int pushSolt = boxSolt - dir;
      if (reach.isReachableBox(pushSolt) &&
          !(board.map[destSolt] & (kWall | kBox))) {
        pushes.emplace_back(boxSolt, dir, board.playerSolt, board);
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

static double costForManhattan(const Board& board, const Push& push) {
  const std::set<int>& goals = board.goals;
  const std::set<int>& boxs = push.data.boxes;

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
  board.boxes.erase(push.boxSolt);
  board.boxes.insert(moveBoxSolt);

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
  board.boxes.insert(push.boxSolt);
  board.boxes.erase(moveBoxSolt);

  board.zobrist.XOR(board.playerZobrists[push.playerSolt]);
  board.zobrist.XOR(board.playerZobrists[movePlayerSolt]);
  board.zobrist.XOR(board.boxZobrists[push.boxSolt]);
  board.zobrist.XOR(board.boxZobrists[moveBoxSolt]);
  // std::cout << "undo push after: " << board.playerSolt << std::endl;
}

static bool checkGameOver(const Board& board) {
  return board.goals == board.boxes;
}

static uint64_t getNowTime() {
  return std::chrono::system_clock::now().time_since_epoch().count();
}

static bool isTunnels(const Push& push, const Board& board) {
  int playerSolt = push.boxSolt - push.dir;
  return ((board.map[playerSolt + 1] & SquareType::kWall) &&
          (board.map[playerSolt - 1] & SquareType::kWall) &&
          ((board.map[push.boxSolt - 1] & SquareType::kWall) ||
           (board.map[push.boxSolt + 1] & SquareType::kWall))) ||
         ((board.map[playerSolt + board.file] & SquareType::kWall) &&
          (board.map[playerSolt - board.file] & SquareType::kWall) &&
          ((board.map[push.boxSolt - board.file] & SquareType::kWall) ||
           (board.map[push.boxSolt + board.file] & SquareType::kWall)));
}

static void astarSearch(Board& board) {
  uint64_t startTime = getNowTime();
  DeadLock dl;
  dl.generate(board);
  int generateNodes = 0;
  int explorNodes = 0;

  std::vector<Push> pushes;
  getPushes(board, pushes);
  if (pushes.empty()) return;

  DynamicData data;
  board.extractDynamicData(data);
  std::set<DynamicData> visited;
  visited.insert(data);
  PriorityQueue<PushPtr, double> frontier;
  for (const auto& push : pushes) {
    ++generateNodes;
    frontier.push(PushPtr(new Push(push)), costForManhattan(board, push));
  }

  while (!frontier.empty()) {
    PushPtr push = frontier.pop();

    board.recoverFromData(push->data);
    ++explorNodes;

    doPush(board, *push);
    // board.print();
    // if (visitedZobrist.find(board.zobrist) != visitedZobrist.end()) {
    //   std::cout << "repeat situation" << std::endl;
    //   continue;
    // }
    // visitedZobrist.insert(board.zobrist);

    DynamicData data;
    board.extractDynamicData(data);
    if (visited.find(data) != visited.end()) {
      continue;
    }

    if (explorNodes % 100000 == 0) {
      board.print();
      board.printForIcon();
      uint64_t endTime = getNowTime();
      printf("generateNodes: %d, explorNodes: %d, spent time: %lu ms\n", generateNodes, explorNodes, (endTime - startTime) / 1000000);
    }

    if (checkGameOver(board)) {
      break;
    }

    visited.insert(data);

    std::vector<Push> pushes;
    getPushes(board, pushes);
    if (pushes.size() == 1) {
      PushPtr pushPtr(new Push(pushes.front()));
      frontier.push(pushPtr, 0);
      // printf("only one push\n");
    } else {
    for (auto p : pushes) {
      ++generateNodes;
      bool isdead = dl.isDeadSolt(p.boxSolt + p.dir);
      if (isdead) continue;
      PushPtr pushPtr(new Push(p));
      bool istunnel = isTunnels(p, board);
      // if (istunnel) printf("tunnel, %d, %d\n", p.boxSolt, p.dir);
      int cost = istunnel ? 0 : costForManhattan(board, p);
      // int cost = costForManhattan(board, p);
      frontier.push(pushPtr, cost);
      // board.print();
      // printf("push item: %d, %d, %d\n", p.boxSolt, p.dir, cost);
    }
    }
  }
  board.print();
  board.printForIcon();
  if (checkGameOver(board)) {
    printf("find best way in Astar search\n");
  } else {
    printf("not find best way in Astar search\n"); 
  }
  uint64_t endTime = getNowTime();
  printf("generateNodes: %d, explorNodes: %d, spent time: %lu ms, speed: %lu nodes/s\n", generateNodes, explorNodes, (endTime - startTime) / 1000000, (uint64_t)((double)explorNodes / (endTime - startTime) * 1000000000));
}

#endif
