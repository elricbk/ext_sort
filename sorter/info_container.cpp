#include "info_container.h"

#include <fstream>

#include <boost/assert.hpp>
#include <boost/foreach.hpp>

info_container_t::info_container_t(size_t ptr_count)
  : m_logger(log4cpp::Category::getRoot())
{
  // FIXME: для поддержки многопоточности нужно резервировать
  // thread_count/(thread_count + 1) от этого количества
  m_records.reserve(ptr_count);
}


namespace {
  typedef record_t* item_type;

  //bool cmp_char_64_r(const record_info_t& first, const record_info_t& second)
  //{
    //for (size_t i = 0; i < 64; ++i) {
      //if (first.key[i] == second.key[i]) continue;
      //return first.key[i] < second.key[i];
    //}
    //return false;
  //}

  void insertion_sort(item_type *array, size_t offset, size_t end) {
      item_type temp;
      for (size_t x=offset; x<end; ++x) {
          for (size_t y=x; y>offset && (*array[y] < *array[y-1]); y--) {
              // FIXME: для массива можно эффективнее
              temp = array[y];
              array[y] = array[y-1];
              array[y-1] = temp;
          }
      }
  }

  void radix_sort_msd_impl(item_type *array, size_t offset, size_t end, size_t idx) {
      size_t x, y;
      item_type value;
      size_t last[256] = { 0 }, pointer[256];

      for (x=offset; x<end; ++x) {
          ++last[array[x]->key[idx]];
      }

      last[0] += offset;
      pointer[0] = offset;
      for (x=1; x<256; ++x) {
          pointer[x] = last[x-1];
          last[x] += last[x-1];
      }

      for (x=0; x<256; ++x) {
          while (pointer[x] != last[x]) {
              value = array[pointer[x]];
              y = value->key[idx];
              while (x != y) {
                  item_type temp = array[pointer[y]];
                  array[pointer[y]++] = value;
                  value = temp;
                  y = value->key[idx];
              }
              array[pointer[x]++] = value;
          }
      }

      if (idx < 64) {
          idx++;
          for (x=0; x<256; ++x) {
              size_t temp = x > 0 ? pointer[x] - pointer[x-1] : pointer[0] - offset;
              if (temp > 64) {
                  radix_sort_msd_impl(array, pointer[x] - temp, pointer[x], idx);
              } else if (temp > 1) {
                  insertion_sort(array, pointer[x] - temp, pointer[x]);
              }
          }
      }
  }

  void radix_sort_msd(item_type* a, size_t n)
  {
      radix_sort_msd_impl(a, 0, n, 0);
  }
} // namespace

void info_container_t::sort()
{
  if (!m_records.empty())
    radix_sort_msd(&m_records.front(), m_records.size());
}

void info_container_t::log_records(const std::string& header) const
{
  m_logger.debugStream() << header;
  BOOST_FOREACH(const record_t* cur, m_records) {
    m_logger.debug("Key: %02x %02x %02x %02x, size: %u", cur->key[0], cur->key[1], cur->key[2], cur->key[3], cur->size);
  }
}

void info_container_t::dump_to_file(const std::string& fname)
{
  m_logger.debugStream() << "Dumping current data to file: " << fname;
  std::ofstream os(fname.c_str(), std::ofstream::binary);
  BOOST_FOREACH(record_t* ri, m_records) {
    os.write(reinterpret_cast<char*>(ri), sizeof(record_t) + ri->size);
  }
  m_records.clear();
}
