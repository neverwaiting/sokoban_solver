#include "parser.h"
#include "solver.h"
#include "zobrist.h"

int main(int argc, char** argv) {

  LevelArray levels;
  getAllLevels(levels);
  // for (auto level : levels) printLevel(level);
  int levelIdx = 0;

  if (argc > 1) {
    levelIdx = atoi(argv[1]) - 1;
  }
  Board board(levels[levelIdx]);
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
