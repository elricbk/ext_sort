#pragma once

#include <fstream>

#include <boost/shared_array.hpp>
#include <boost/throw_exception.hpp>

#include "log4cpp/Category.hh"

#include "common/test.h"
#include "common/record_info.h"

class output_buffer_t {
public:
  output_buffer_t(const std::string& outfile, const std::string& infile, size_t ram_size)
    : m_logger(log4cpp::Category::getRoot())
    , m_max_records(ram_size/sizeof(record_info_t))
    , m_idx(0)
    , m_records(new record_info_t[m_max_records])
    , m_infile(infile.c_str(), std::ifstream::binary)
    , m_outfile(outfile.c_str(), std::ofstream::binary)
  {
  }

  void add(const record_info_t& rec)
  {
    m_logger.debug("Adding record to cache");
    m_records[m_idx] = rec;
    m_idx++;
    if (m_idx == m_max_records) {
      save();
      m_idx = 0;
    }
  }

  void dump()
  {
    if (m_idx != 0) {
      save();
      m_idx = 0;
    }
  }

private:
  void save()
  {
    m_logger.debug("Saving %u records", m_idx);
    // использование m_idx позволяет сохранять неполный буффер
    for (size_t i = 0; i < m_idx; ++i)
      save_record(m_records[i]);
  }

  void save_record(const record_info_t& rec)
  {
    // FIXME: просмотреть все проверки, где должно быть good(), где fail() или bad()
    m_infile.seekg(rec.offset);
    if (m_infile.fail() || m_infile.bad())
      BOOST_THROW_EXCEPTION(std::runtime_error("Unable to seek to offset in input file"));

    test test_rec;
    m_infile.read(reinterpret_cast<char*>(&test_rec), sizeof(test));
    if (m_infile.fail() || m_infile.bad())
      BOOST_THROW_EXCEPTION(std::runtime_error("Unable to read data from input file"));
    m_logger.debug("Found record, key=%02x %02x %02x %02x, size=%u", test_rec.key[0], test_rec.key[1], test_rec.key[2], test_rec.key[3], test_rec.size);

    // проверяем, что первые 8 байт ключа совпадают
    BOOST_ASSERT((*reinterpret_cast<const uint64_t*>(test_rec.key)) == (*reinterpret_cast<const uint64_t*>(rec.key)));

    m_outfile.write(reinterpret_cast<char*>(&test_rec), sizeof(test_rec));
    if (!m_outfile.good())
      BOOST_THROW_EXCEPTION(std::runtime_error("Unable to write data to output file"));

    char buffer[1024*1024]; // FIXME: должно быть в параметрах, наверное, может не хватить памяти
    uint64_t data_size = test_rec.size;
    while (data_size > 0) {
      uint64_t chunk_size = std::min(static_cast<uint64_t>(1024*1024), data_size);
      m_infile.read(buffer, chunk_size);
      if (m_infile.fail() || m_infile.bad())
        BOOST_THROW_EXCEPTION(std::runtime_error("Unable to read data from input file"));
      m_outfile.write(buffer, chunk_size);
      if (!m_outfile.good())
        BOOST_THROW_EXCEPTION(std::runtime_error("Unable to write data to output file"));
      data_size -= chunk_size;
    }
  }

private:
  log4cpp::Category& m_logger;
  size_t m_max_records;
  size_t m_idx;
  // FIXME: boost::array or c++11 maybe?
  boost::shared_array<record_info_t> m_records;
  std::ifstream m_infile;
  std::ofstream m_outfile;
};
