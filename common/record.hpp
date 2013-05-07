#pragma once

#include <sstream>
#include <iomanip>

#include "log4cpp/Category.hh"

#include "test.hpp"

typedef test record_t;

inline
record_t make_record(uint64_t flags = 0, uint64_t crc = 0, uint64_t size = 0)
{
  record_t res;
  res.flags = flags;
  res.crc = crc;
  res.size = size;
  return res;
}

inline
bool operator<(const record_t& lhs, const record_t& rhs)
{
  for (size_t i = 0; i < 64; ++i) {
    if (lhs.key[i] == rhs.key[i]) continue;
    return lhs.key[i] < rhs.key[i];
  }
  return false;
}

inline
log4cpp::CategoryStream& operator<< (log4cpp::CategoryStream& os, const record_t& rec)
{
  std::ostringstream ss;
  ss << "record<key=\"" << std::hex;
  for (int i = 0; i < 3; ++i)
    ss << std::setw(2) << std::setfill('0') << static_cast<uint16_t>(rec.key[i]) << " ";
  ss << std::setw(2) << std::setfill('0') << static_cast<uint16_t>(rec.key[3]) << "...\",";
  ss << " size=" << std::dec << rec.size << ">";
  os << ss.str();
  return os;
}
