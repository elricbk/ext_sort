#pragma once

#include <cstdio>
#include <fstream>

#include <boost/shared_array.hpp>
#include <boost/throw_exception.hpp>

#include "log4cpp/Category.hh"

#include "common/record_info.h"

class input_buffer_t {
public:
  input_buffer_t (const std::string& fname, size_t ram_size)
    : m_logger(log4cpp::Category::getRoot())
    , m_file_name(fname)
    , m_stream(fname.c_str(), std::ifstream::binary)
    , m_max_records(ram_size/sizeof(record_info_t))
    , m_idx(0)
    , m_rec_count(0)
    , m_records(new record_info_t[m_max_records])
  {
    m_logger.debugStream() << "Input file name " << fname << ", available memory " << ram_size;
    m_logger.debug("Max records: %u", m_max_records);
    BOOST_ASSERT(m_max_records > 0);
  }

  ~input_buffer_t()
  {
    if (remove(m_file_name.c_str()) != 0)
      m_logger.errorStream() << "Error while deleting file " << m_file_name; 
  }

  bool has_cached_data() const { return (m_idx < m_rec_count); }
  bool eof() const { return m_stream.eof(); }
  const record_info_t& peek() const { return m_records[m_idx]; }
  record_info_t pop() { record_info_t res = m_records[m_idx]; m_idx++; return res; }

  void load_data()
  {
    m_stream.read(reinterpret_cast<char*>(m_records.get()), sizeof(record_info_t)*m_max_records);
    std::streamsize bytes_read = m_stream.gcount();
    m_logger.debug("Read %d bytes from file", bytes_read);
    if ((bytes_read != sizeof(record_info_t)*m_max_records) && !m_stream.eof())
      BOOST_THROW_EXCEPTION(std::runtime_error("Some error while reading data from temporary file"));
    if (bytes_read % sizeof(record_info_t))
      BOOST_THROW_EXCEPTION(std::runtime_error("Incorrect amount of bytes was read from temporary file"));
    m_idx = 0;
    m_rec_count = bytes_read / sizeof(record_info_t);
    m_logger.debug("Record count after read: %u", m_rec_count);
  }

private:
  log4cpp::Category& m_logger;
  std::string m_file_name;
  std::ifstream m_stream;
  size_t m_max_records;
  size_t m_idx;
  size_t m_rec_count;
  boost::shared_array<record_info_t> m_records;
};
