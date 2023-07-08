#include "zobrist.h"

namespace sokoban {
// RC4密码流
RC4::RC4() : x_(0), y_(0) {
  int i = 0;
  int j = 0;
  for (i = 0; i < state_.size(); ++i) {
    state_[i] = i;
  }
  for (i = 0; i < state_.size(); ++i) {
    j = (j + state_[i]) & 0xff;
    std::swap(state_[i], state_[j]);
  }
}

uint8_t RC4::NextByte() {
  x_ = (x_ + 1) & 0xff;
  y_ = (y_ + state_[x_]) & 0xff;
  std::swap(state_[x_], state_[y_]);
  return state_[(state_[x_] + state_[y_]) & 0xff];
}

uint32_t RC4::NextLong() {
  return NextByte() + (NextByte() << 8) + (NextByte() << 16) + (NextByte() << 24);
}

Zobrist::Zobrist() : key_(0), lock1_(0), lock2_(0) {
}

Zobrist::Zobrist(RC4& rc4) {
  key_ = rc4.NextLong();
  lock1_ = rc4.NextLong();
  lock2_ = rc4.NextLong();
}

void Zobrist::Reset() {
  key_ = lock1_ = lock2_ = 0;
}

void Zobrist::XOR(const Zobrist& rhs) {
  key_ ^= rhs.key_;
  lock1_ ^= rhs.lock1_;
  lock2_ ^= rhs.lock2_;
}

uint32_t Zobrist::key() const {
  return key_;
}

uint32_t Zobrist::lock1() const {
  return lock1_;
}

uint32_t Zobrist::lock2() const{
  return lock2_;
}

} // namespace sokoban
