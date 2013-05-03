#pragma once

#include <vector>
#include <string>

#include "log4cpp/Category.hh"

#include "common/test.h"
#include "common/record_info.h"

class info_container_t
{
public:
  info_container_t(size_t ram_size);
  void add(const test& record, uint64_t offset);
  bool is_full() const { return m_records.size() == m_max_records; }
  bool is_empty() const { return m_records.empty(); }
  void sort();
  void dump_to_file(const std::string& fname);
  void log_records(const std::string& header) const;

private:
  log4cpp::Category& m_logger;
  size_t m_max_records;
  std::vector<record_info_t> m_records;
};
