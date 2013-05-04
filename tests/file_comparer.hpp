#include <cstdio>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>
#include <boost/throw_exception.hpp>

#include <log4cpp/Category.hh>

#include "common/record_info.h"

class file_comparer_t
{
public:
  file_comparer_t(const std::string& fname_orig = "original.test.dat", const std::string& fname_sort = "sorted.test.dat")
    : m_logger(log4cpp::Category::getRoot())
    , m_fname_orig(fname_orig)
    , m_fname_sort(fname_sort) {}

  ~file_comparer_t()
  {
    if (remove(m_fname_orig.c_str()) != 0)
      m_logger.errorStream() << "Error while deleting file " << m_fname_orig;
    if (remove(m_fname_sort.c_str()) != 0)
      m_logger.errorStream() << "Error while deleting file " << m_fname_sort;
  }

  const std::string& fname_orig() { return m_fname_orig; }

  const std::string& fname_sort() { return m_fname_sort; }

  bool file_is_sorted()
  {
    if (!m_rec_sort)
      m_rec_sort = records_from_file(m_fname_sort);

    for (size_t i = 0; i < m_rec_sort->size() - 1; ++i) {
      if (!(m_rec_sort->at(i) < m_rec_sort->at(i + 1))) {
        return false;
      }
    }
    return true;
  }

  bool records_are_equal()
  {
    if (!m_rec_orig) {
      m_rec_orig = records_from_file(m_fname_orig);
      std::sort(m_rec_orig->begin(), m_rec_orig->end());
    }
    if (!m_rec_sort)
      m_rec_sort = records_from_file(m_fname_sort);

    m_logger.info("Original file record count: %u", m_rec_orig->size());
    m_logger.info("Sorted file record count: %u", m_rec_sort->size());

    if (m_rec_orig->empty()) {
      m_logger.warn("No entries found in original file");
      return false;
    }

    if (m_rec_sort->empty()) {
      m_logger.warn("No entries found in sorted file");
      return false;
    }

    if (m_rec_sort->size() != m_rec_orig->size()) {
      m_logger.warn("Record count in files differ");
    }

    for (size_t i = 0; i < m_rec_orig->size(); ++i) {
      if (!keys_are_equal(m_rec_orig->at(i), m_rec_sort->at(i))) {
        m_logger.warn("Keys differ at index %u", i);
        return false;
      }
    }
    return true;
  }


private:
  typedef boost::shared_ptr<std::vector<record_info_t> > record_list_t;

  record_list_t records_from_file(const std::string& fname)
  {
    std::ifstream is(fname.c_str(), std::ifstream::binary);
    record_list_t result(new std::vector<record_info_t>());
    while (is.good())
    {
      test cur_test;
      size_t offset = is.tellg();
      is.read(reinterpret_cast<char*>(&cur_test), sizeof(cur_test));
      std::streamsize bytes_read = is.gcount();
      if (bytes_read != sizeof(cur_test))
        BOOST_THROW_EXCEPTION(std::runtime_error("Unable to read test struct from input file"));
      is.ignore(cur_test.size);
      m_logger.debug("%s: record key=%02x %02x %02x %02x, size=%u", fname.c_str(), cur_test.key[0], cur_test.key[1], cur_test.key[2], cur_test.key[3], cur_test.size);
      result->push_back(record_info_t(cur_test, offset));
    }
    return result;
  }

  static bool keys_are_equal(const record_info_t& first, const record_info_t& second)
  {
    return std::memcmp(first.key, second.key, 64) == 0;
  }

private:
  log4cpp::Category& m_logger;
  std::string m_fname_orig;
  std::string m_fname_sort;
  record_list_t m_rec_orig;
  record_list_t m_rec_sort;
};
