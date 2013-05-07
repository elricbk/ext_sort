#pragma once

#include <fstream>

#include <boost/scoped_array.hpp>
#include <boost/throw_exception.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/stream.hpp>

#include "log4cpp/Category.hh"

#include "common/record.hpp"

namespace io = boost::iostreams;

class output_buffer_t {
public:
  output_buffer_t(const std::string& outfile, size_t ram_size)
    : m_logger(log4cpp::Category::getRoot())
    , m_ram_size(ram_size)
    , m_idx(0)
    , m_data(new char[m_ram_size])
    , m_outfile(outfile.c_str(), std::ofstream::binary)
  {
    if (!m_outfile.is_open())
      BOOST_THROW_EXCEPTION(std::runtime_error("Unable to open output file"));
  }

  void add(const record_t* rec)
  {
    BOOST_ASSERT(rec);
    m_logger.debugStream() << "Adding: " <<  *rec <<
      ", space available: m_ram_size=" << m_ram_size << " m_idx=" << m_idx;
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
    m_outfile.write(m_data.get(), m_idx);
    if (!m_outfile.good())
      BOOST_THROW_EXCEPTION(std::runtime_error("Error writing data to output file"));
    m_idx = 0;
  }

private:
  log4cpp::Category& m_logger;
  size_t m_ram_size;
  size_t m_idx;
  boost::scoped_array<char> m_data;
  io::stream<io::file_sink> m_outfile;
};
