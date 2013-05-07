#pragma once

#include <istream>

#include <boost/scoped_array.hpp>
#include <boost/noncopyable.hpp>

#include "log4cpp/Category.hh"

#include "common/record.hpp"

//! Класс для работы с буфером данных заданного размера, содержащих данные с заголовками типа record_t
class input_buffer_t: boost::noncopyable {
public:
  input_buffer_t (std::istream& stream, size_t ram_size);

  //! Получить указатель на первый элемент данных в буфере
  /** Можно вызывать только если has_cached_data() == true */
  const record_t* peek() const;

  //! Перейти к следуещему элементу данных
  /** Можно вызывать только если has_cached_data() == true */
  void pop();

  //! Проверка на то, что в буфере есть кэшированные данные
  bool has_cached_data() const { return m_has_cached_data; }

  //! Проверка на то, что все данные из потока вычитаны
  bool eof() const { return m_stream.eof(); }

  //! Загрузка очередной порции данных из потока
  void load_data();

  //! Формирование указателей на все элементы данных в буфере
  /** Формирование жадное:после вызова этого метода has_cached_data() вернёт false */
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
