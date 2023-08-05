#ifndef ZOBRIST_H_
#define ZOBRIST_H_

#include <inttypes.h>
#include <array>

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
  Zobrist(const Zobrist&);
	void Reset();
	void XOR(const Zobrist& rhs);
  bool operator==(const Zobrist& rhs) const;
  bool operator!=(const Zobrist& rhs) const;
  bool operator<(const Zobrist& rhs) const;

  using Array = std::array<uint32_t, 4>;
private:
  Array keys;
};

#endif // #ifdef ZOBRIST_H_
