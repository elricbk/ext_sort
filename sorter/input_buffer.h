#pragma once

#include <cstdio>
#include <fstream>

#include <boost/shared_array.hpp>
#include <boost/throw_exception.hpp>

#include "log4cpp/Category.hh"

#include "common/record.h"

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
    if (m_ram_size < sizeof(record_t))
      BOOST_THROW_EXCEPTION(std::runtime_error("Available memory is less than record size"));
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

    size_t left = m_data_available - m_idx;
    if (left > 0)
      std::memmove(m_data.get(), m_data.get() + m_idx, left);
    size_t bytes_to_read = m_ram_size - left;
    m_stream.read(m_data.get() + left, bytes_to_read);
    std::streamsize bytes_read = m_stream.gcount();
    m_logger.debug("Read %d bytes from file", bytes_read);
    if ((bytes_read != bytes_to_read) && !m_stream.eof())
      BOOST_THROW_EXCEPTION(std::runtime_error("Some error while reading data from temporary file"));
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
    ptrs->push_back(ptr);
    idx += ptr->size + sizeof(record_t);

    while (check_memory(idx)) {
      ptr = reinterpret_cast<record_t*>(m_data.get() + idx);
      ptrs->push_back(ptr);
      idx += ptr->size + sizeof(record_t);
    }
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
    if (rec->size > m_ram_size) {
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

// Обёртка над входным файлом, нужна чтобы буфер было проще тестировать
class input_file_t {
public:
  input_file_t (const std::string& fname, size_t ram_size, bool rm_file = false)
    : m_logger(log4cpp::Category::getRoot())
    , m_delete_file(rm_file)
    , m_file_name(fname)
    , m_stream(m_file_name.c_str(), std::ifstream::binary)
    , m_buffer(m_stream, ram_size) {}

  ~input_file_t()
  {
    m_stream.close();
    if (m_delete_file) {
      m_logger.debugStream() << "Removing file " << m_file_name;
      if (remove(m_file_name.c_str()) != 0)
        m_logger.errorStream() << "Error while deleting file " << m_file_name; 
    }
  }

  input_buffer_t& buffer() { return m_buffer; }

private:
  log4cpp::Category& m_logger;
  bool m_delete_file;
  std::string m_file_name;
  std::ifstream m_stream;
  input_buffer_t m_buffer;
};
