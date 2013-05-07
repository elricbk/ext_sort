#include <fstream>

#include <boost/assert.hpp>
#include <boost/scoped_array.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/noncopyable.hpp>
#include <boost/throw_exception.hpp>

#include "common/record.hpp"

class generator_t: boost::noncopyable {
public:
  generator_t (const std::string& fname, size_t file_size, size_t data_size, bool random_seed = false, bool random_data = false)
    : m_logger(log4cpp::Category::getRoot())
    , m_fname(fname)
    , m_file_size(file_size)
    , m_data_size(data_size)
    , m_dist(0, std::numeric_limits<unsigned char>::max())
    , m_size_dist(0, m_data_size)
    , m_random_data(random_data)
  {
    if (random_seed)
      m_gen.seed(std::time(0));
  }

  void generate_data()
  {
    std::ofstream os(m_fname.c_str(), std::ios::binary);
    size_t total_size = 0;
    // FIXME: опасно такое оставлять для больших размеров данных
    boost::scoped_array<char> sa(new char[m_data_size]);
    if (!m_random_data)
      std::memset(sa.get(), m_data_size, 0);

    while (total_size < m_file_size) {
      record_t res = generate_test(); 
      BOOST_ASSERT(res.size <= m_data_size);
      m_logger.debugStream() << "Generated record: " << res;
      os.write(reinterpret_cast<char*>(&res), sizeof(res));
      if (m_random_data)
        generate_data(sa.get(), res.size);
      os.write(sa.get(), res.size);
      if (!os.good())
        BOOST_THROW_EXCEPTION(std::runtime_error("Unable to write to output file"));
      total_size += sizeof(res) + res.size;
    }
  }

private:
  record_t generate_test()
  {
    test res;
    for (size_t i = 0; i < 64; ++i)
      res.key[i] = m_dist(m_gen);
    res.size = m_size_dist(m_gen);
    return res;
  }

  void generate_data(char* data, uint64_t size)
  {
    for (size_t i = 0; i < size; ++i)
      data[i] = m_dist(m_gen);
  }

private:
  log4cpp::Category& m_logger;
  std::string m_fname;
  size_t m_file_size;
  size_t m_data_size;
  boost::random::mt19937 m_gen;
  boost::random::uniform_int_distribution<unsigned char> m_dist;
  boost::random::uniform_int_distribution<uint64_t> m_size_dist;
  bool m_random_data;
};

