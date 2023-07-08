#ifndef ZOBRIST_H_
#define ZOBRIST_H_

#include <inttypes.h>
#include <array>

namespace sokoban {
// RC4密码流
class RC4 {
public:
	RC4();
	uint8_t NextByte();
	uint32_t NextLong();

private:
	int x_;
	int y_;
	std::array<uint8_t, 256> state_;
};

class Zobrist {
public:
	Zobrist();
	Zobrist(RC4& rc4);
  Zobrist(const Zobrist&) = default;
	void Reset();
	void XOR(const Zobrist& rhs);
  uint32_t key() const;
  uint32_t lock1() const;
  uint32_t lock2() const;

private:
	uint32_t key_;
	uint32_t lock1_;
	uint32_t lock2_;
};

} // namespace sokoban

#endif // #ifdef ZOBRIST_H_
