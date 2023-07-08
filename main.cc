#include "parser.h"
#include "solver.h"

int main(int argc, char** argv) {
  LevelArray levels;
  getAllLevels(levels);
  // for (auto level : levels) printLevel(level);
  Board board(levels[0]);
  // std::vector<Push> pushes;
  // getPushes(board, pushes);
  // for (const auto& push : pushes) {
  //   std::cout << "===" << std::endl;
  //   board.print();
  //   doPush(board, push);
  //   std::cout << "+++" << std::endl;
  //   board.print();
  //   std::vector<Push> ps;
  //   getPushes(board, ps);
  //   undoPush(board, push);
  //   std::cout << "---" << std::endl;
  //   board.print();
  // }
  astarSearch(board);

  return 0;
}
