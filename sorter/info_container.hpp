#pragma once

#include <vector>
#include <string>

#include "log4cpp/Category.hh"

#include "common/record.hpp"

class info_container_t {
public:
  explicit info_container_t(size_t ptr_count);
  std::vector<record_t*>* pointers() { return &m_records; }
  void sort();
  void dump_to_file(const std::string& fname);
  void log_records(const std::string& header) const;

private:
  log4cpp::Category& m_logger;
  std::vector<record_t*> m_records;
};
