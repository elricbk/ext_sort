#pragma once

#include <cstdio>

#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/stream.hpp>

#include "log4cpp/Category.hh"

#include "input_buffer.hpp"

namespace io = boost::iostreams;

//! Обёртка над входным файлом, чтобы буфер можно было тестировать отдельно
/** При передаче соответствующего параметра, удаляет обёртываемый файл при уничтожении  */
class input_file_t {
public:
  input_file_t (const std::string& fname, size_t ram_size, bool rm_file = false)
    : m_logger(log4cpp::Category::getRoot())
    , m_delete_file(rm_file)
    , m_file_name(fname)
    , m_sbuf(m_file_name)
    , m_buffer(m_sbuf, ram_size) {}

  ~input_file_t()
  {
    m_sbuf.close();
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
  io::stream<io::file_source> m_sbuf;
  input_buffer_t m_buffer;
};
