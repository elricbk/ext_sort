#include <fstream>

#include <boost/scoped_array.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include "common/test.h"

class generator_t {
public:
  generator_t (const std::string& fname, size_t file_size, size_t data_size)
    : m_logger(log4cpp::Category::getRoot())
    , m_fname(fname)
    , m_file_size(file_size)
    , m_data_size(data_size)
    , m_dist(0, std::numeric_limits<unsigned char>::max())
    , m_size_dist(0, m_data_size) {}

  void generate_data()
  {
    std::ofstream os(m_fname.c_str(), std::ios::binary);
    size_t total_size = 0;
    // FIXME: опасно такое оставлять для больших размеров данных
    boost::scoped_array<char> sa(new char[m_data_size]);
    std::memset(sa.get(), m_data_size, 0);

    while (total_size < m_file_size) {
      test res = generate_test(); 
      m_logger.debug("Key: %02x %02x %02x %02x, Size: %u", res.key[0], res.key[1], res.key[2], res.key[3], res.size);
      os.write(reinterpret_cast<char*>(&res), sizeof(res));
      os.write(sa.get(), res.size);
      total_size += sizeof(res) + res.size;
    }
  }

private:
  test generate_test()
  {
    test res;
    for (size_t i = 0; i < 64; ++i)
      res.key[i] = m_dist(m_gen);
    res.size = m_size_dist(m_gen);
    return res;
  }

private:
  log4cpp::Category& m_logger;
  std::string m_fname;
  size_t m_file_size;
  size_t m_data_size;
  boost::random::mt19937 m_gen;
  boost::random::uniform_int_distribution<unsigned char> m_dist;
  boost::random::uniform_int_distribution<uint64_t> m_size_dist;
};

