#include "zobrist.h"
#include <algorithm>
#include <iostream>

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

Zobrist::Zobrist() {
  std::fill(keys.begin(), keys.end(), 0);
}

Zobrist::Zobrist(const Zobrist& rhs) {
  std::copy(rhs.keys.begin(), rhs.keys.end(), keys.begin());
}

Zobrist::Zobrist(RC4& rc4) {
  for (auto it = keys.begin(); it != keys.end(); ++it) {
    *it = rc4.NextLong();
  }
}

void Zobrist::Reset() {
  std::fill(keys.begin(), keys.end(), 0);
}

void Zobrist::XOR(const Zobrist& rhs) {
  auto it = keys.begin();
  auto itr = rhs.keys.cbegin();
  for (; it != keys.end(); ++it, ++itr) {
    *it ^= *itr;
  }
}

bool Zobrist::operator==(const Zobrist& rhs) const {
  for (auto it = keys.begin(), itr = rhs.keys.cbegin();
      it != keys.end(); ++it, ++itr) {
  }
  return std::equal(keys.begin(), keys.end(), rhs.keys.begin());
}

bool Zobrist::operator!=(const Zobrist& rhs) const {
  return !operator==(rhs);
}

bool Zobrist::operator<(const Zobrist& rhs) const {
  return keys < rhs.keys;
}
