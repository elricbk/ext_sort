#pragma once

struct record_t {
  unsigned char        key[64];
  uint64_t        flags;
  uint64_t        crc;
  uint64_t        size;

  record_t(uint64_t flags = 0, uint64_t crc = 0, uint64_t size = 0)
  {
    this->flags = flags;
    this->crc = crc;
    this->size = size;
  }

  bool operator<(const record_t& rhs) const
  {
    for (size_t i = 0; i < 64; ++i) {
      if (key[i] == rhs.key[i]) continue;
      return key[i] < rhs.key[i];
    }
    return false;
  }
};
