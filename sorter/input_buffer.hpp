#pragma once

#include <istream>

#include <boost/scoped_array.hpp>
#include <boost/noncopyable.hpp>

#include "log4cpp/Category.hh"

#include "common/record.hpp"

class input_buffer_t: boost::noncopyable {
public:
  input_buffer_t (std::istream& stream, size_t ram_size);

  const record_t* peek() const;
  void pop();
  bool has_cached_data() const { return m_has_cached_data; }
  bool eof() const { return m_stream.eof(); }
  void load_data();
  void get_pointers(std::vector<record_t*> * ptrs);

private:
  bool check_memory(size_t idx);

private:
  log4cpp::Category& m_logger;
  std::istream& m_stream;
  size_t m_ram_size;
  bool m_has_cached_data;
  boost::scoped_array<char> m_data;
  size_t m_idx;
  size_t m_data_available;
};

inline
const record_t* input_buffer_t::peek() const
{
  BOOST_ASSERT(m_has_cached_data);
  return reinterpret_cast<const record_t*>(m_data.get() + m_idx);
}
