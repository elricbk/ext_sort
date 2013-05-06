#pragma once

#include <istream>

#include <boost/shared_array.hpp>
#include <boost/throw_exception.hpp>

#include "log4cpp/Category.hh"

#include "common/record.h"

// FIXME: explicit constructors where possible?

class input_buffer_t {
public:
  input_buffer_t (std::istream& stream, size_t ram_size)
    : m_logger(log4cpp::Category::getRoot())
    , m_stream(stream)
    , m_ram_size(ram_size)
    , m_has_cached_data(false)
    , m_data(new char[m_ram_size])
    , m_idx(0)
    , m_data_available(0)
  {
    m_logger.debug("input_buffer_t: m_ram_size=%u", m_ram_size);
    if (m_ram_size < sizeof(record_t)) {
      m_logger.error("Available ram (%u) is less than record size (%u)", m_ram_size, sizeof(record_t));
      BOOST_THROW_EXCEPTION(std::runtime_error("Available memory is less than record size"));
    }
  }

  bool has_cached_data() const { return m_has_cached_data; }
  bool eof() const { return m_stream.eof(); }

  const record_t* peek() const
  { 
    BOOST_ASSERT(m_has_cached_data);
    return reinterpret_cast<const record_t*>(m_data.get() + m_idx);
  }

  void pop()
  {
    BOOST_ASSERT(m_has_cached_data);
    const record_t* rec = peek();
    m_logger.debug("Popping record key=%02x %02x %02x %02x, size=%u", rec->key[0], rec->key[1], rec->key[2], rec->key[3], rec->size);
    m_idx += rec->size + sizeof(record_t);
    m_has_cached_data = check_memory(m_idx);
    if (m_has_cached_data) {
      const record_t* rec = peek();
      m_logger.debug("Record after pop key=%02x %02x %02x %02x, size=%u", rec->key[0], rec->key[1], rec->key[2], rec->key[3], rec->size);
    }
  }

  void load_data()
  {
    BOOST_ASSERT(m_idx < m_ram_size);

    m_logger.debug("Loading data, m_idx=%u m_data_available=%u", m_idx, m_data_available);
    size_t left = m_data_available - m_idx;
    if (left > 0)
      std::memmove(m_data.get(), m_data.get() + m_idx, left);
    size_t bytes_to_read = m_ram_size - left;
    size_t bytes_read = 0;
    while (bytes_to_read > 0) {
      std::streamsize btr = std::numeric_limits<std::streamsize>::max() < bytes_to_read ?
        std::numeric_limits<std::streamsize>::max() : bytes_to_read;
      m_stream.read(m_data.get() + left, btr);
      std::streamsize br = m_stream.gcount();
      m_logger.debug("Read %d bytes from file (tried to read %u)", br, btr);
      if ((br != btr) && !m_stream.eof())
        BOOST_THROW_EXCEPTION(std::runtime_error("Some error while reading data from temporary file"));
      bytes_to_read = m_stream.eof() ? 0 : bytes_to_read - br;
      bytes_read += br;
    }
    m_idx = 0;
    m_data_available = left + bytes_read;
    m_has_cached_data = check_memory(m_idx);
  }

  void get_pointers(std::vector<record_t*> * ptrs)
  {
    BOOST_ASSERT(ptrs);
    ptrs->clear();
    if (!m_has_cached_data)
      return;
    size_t idx = m_idx;
    record_t* ptr = reinterpret_cast<record_t*>(m_data.get() + idx);

    while (idx + ptr->size + sizeof(record_t) <= m_data_available) {
      ptrs->push_back(ptr);
      idx += ptr->size + sizeof(record_t);
      ptr = reinterpret_cast<record_t*>(m_data.get() + idx);
    }

    if ((idx + sizeof(record_t) <= m_data_available) && (ptr->size + sizeof(record_t) > m_ram_size)) {
      m_logger.error("Record size (%u) is greater than available memory (%u), unable to continue", ptr->size, m_ram_size);
      BOOST_THROW_EXCEPTION(std::runtime_error("Record size is greater than available memory"));
    }

    m_idx = idx;
  }

private:
  bool check_memory(size_t idx)
  {
    m_logger.debug("check_memory: m_data_available=%u, idx=%u", m_data_available, idx);
    BOOST_ASSERT(idx <= m_data_available);
    size_t left  = m_data_available - idx;
    if (left < sizeof(record_t)) {
      return false;
    }
    const record_t* rec = reinterpret_cast<const record_t*>(m_data.get() + idx);
    if (rec->size + sizeof(record_t) > m_ram_size) {
      m_logger.error("Record size (%u) is greater than available memory (%u), unable to continue", rec->size, m_ram_size);
      BOOST_THROW_EXCEPTION(std::runtime_error("Record size is greater than available memory"));
    }
    if (left < rec->size + sizeof(record_t)) {
      if (m_stream.eof())
        BOOST_THROW_EXCEPTION(std::runtime_error("Not enough data for current record in file"));
      return false;
    }
    return true;
  }

private:
  log4cpp::Category& m_logger;
  std::istream& m_stream;
  size_t m_ram_size;
  bool m_has_cached_data;
  boost::shared_array<char> m_data;
  size_t m_idx;
  size_t m_data_available;
};
