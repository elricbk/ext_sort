#pragma once

#include <fstream>

#include <boost/shared_array.hpp>
#include <boost/throw_exception.hpp>

#include "log4cpp/Category.hh"

#include "common/record.h"

class output_buffer_t {
public:
  output_buffer_t(const std::string& outfile, size_t ram_size)
    : m_logger(log4cpp::Category::getRoot())
    , m_ram_size(ram_size)
    , m_idx(0)
    , m_data(new char[m_ram_size])
    , m_outfile(outfile.c_str(), std::ofstream::binary)
  {
  }

  void add(const record_t* rec)
  {
    BOOST_ASSERT(rec);
    m_logger.debug("Adding record key=%02x %02x %02x %02x, size=%u", rec->key[0], rec->key[1], rec->key[2], rec->key[3], rec->size);
    m_logger.debug("Space available: m_ram_size=%u m_idx=%u", m_ram_size, m_idx);
    size_t total_size = rec->size + sizeof(record_t);
    // FIXME: это вообще-то можно обойти поблочным копированием
    if (total_size > m_ram_size)
      BOOST_THROW_EXCEPTION(std::runtime_error("Total record size is greater than memory available"));
    if ((m_ram_size - m_idx) < total_size)
      save();
    std::memcpy(m_data.get() + m_idx, rec, total_size);
    m_idx += total_size;
  }

  void dump()
  {
    if (m_idx != 0)
      save();
  }

private:
  void save()
  {
    m_logger.debug("Saving %u bytes", m_idx);
    m_outfile.write(m_data.get(), m_idx); // FIXME: проверка ошибок?
    m_idx = 0;
  }

private:
  log4cpp::Category& m_logger;
  size_t m_ram_size;
  size_t m_idx;
  boost::shared_array<char> m_data;
  std::ofstream m_outfile;
};
