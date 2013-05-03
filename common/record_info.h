#pragma once

#include "test.h"

class record_info_t
{
public:
  record_info_t() {}

  record_info_t(const test& record, uint64_t offset)
  {
    std::memcpy(key, record.key, 64);
    this->offset = offset;
  }

  bool operator<(const record_info_t& rhs) const
  {
    for (size_t i = 0; i < 64; ++i) {
      if (key[i] == rhs.key[i]) continue;
      return key[i] < rhs.key[i];
    }
    return false;
  }

  unsigned char key[64];
  uint64_t offset;
};
