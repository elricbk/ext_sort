#include <iostream>
#include <fstream>

#include <boost/shared_array.hpp>
#include <boost/program_options.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include "log4cpp/Category.hh"
#include "log4cpp/Appender.hh"
#include "log4cpp/FileAppender.hh"
#include "log4cpp/OstreamAppender.hh"
#include "log4cpp/Layout.hh"
#include "log4cpp/PatternLayout.hh"
#include "log4cpp/Priority.hh"

#include "common/test.h"

class test_generator
{
public:
  test_generator(size_t max_data_size = 4*1024)
    : m_dist(0, std::numeric_limits<unsigned char>::max())
    , m_size_dist(0, max_data_size) {}

  test operator() ()
  {
    test res;
    for (size_t i = 0; i < 64; ++i)
      res.key[i] = m_dist(m_gen);
    res.size = m_size_dist(m_gen);
    return res;
  }

private:
  boost::random::mt19937 m_gen;
  boost::random::uniform_int_distribution<unsigned char> m_dist;
  boost::random::uniform_int_distribution<uint64_t> m_size_dist;
};

void generate_data(const std::string& fname, size_t file_size)
{
  std::ofstream os(fname.c_str(), std::ios::binary);
  size_t total_size = 0;
  test_generator gen;
  log4cpp::Category& logger = log4cpp::Category::getRoot();
  // FIXME: опасно такое оставлять для больших размеров данных
  boost::shared_array<char> sa(new char[4*1024]);
  std::memset(sa.get(), 4*1024, 0);

  while (total_size < file_size) {
    test res = gen(); 
    logger.debug("Key: %02x %02x %02x %02x, Size: %u", res.key[0], res.key[1], res.key[2], res.key[3], res.size);
    os.write(reinterpret_cast<char*>(&res), sizeof(res));
    os.write(sa.get(), res.size);
    total_size += sizeof(res) + res.size;
  }
}

void init_logging(bool verbose)
{
  log4cpp::PatternLayout* layout1 = new log4cpp::PatternLayout();
  layout1->setConversionPattern("%d [%p] %m%n");
  log4cpp::Appender *appender1 = new log4cpp::OstreamAppender("console", &std::cout);
  appender1->setLayout(layout1);

  log4cpp::Category& root = log4cpp::Category::getRoot();
  root.setPriority(verbose ? log4cpp::Priority::DEBUG : log4cpp::Priority::WARN);
  root.addAppender(appender1);

  if (verbose) {
    log4cpp::PatternLayout* layout2 = new log4cpp::PatternLayout();
    layout2->setConversionPattern("%d [%p] %m%n");
    log4cpp::Appender *appender2 = new log4cpp::FileAppender("default", "generator.log");
    appender2->setLayout(layout2);
    root.addAppender(appender2);
  }
}

int main(int ac, const char *av[])
{
  namespace po = boost::program_options;

  size_t file_size;
  std::string file_name;
  bool verbose;
  bool randomize;

  po::options_description desc("Options for test data generator");
  desc.add_options()
    ("help,h", "produce help message")
    ("size,s", po::value<size_t>(&file_size)->default_value(1024), "set approximate output file size in Mb, generation will stop when this size is exceeded")
    ("output,o", po::value<std::string>(&file_name)->default_value("test.dat"), "set output file name")
    ("verbose,v", po::value<bool>(&verbose)->zero_tokens(), "verbose logging")
    ("randomize-data", po::value<bool>(&randomize)->default_value(false), "set random seed for data generator")
    ;

  po::variables_map vm;
  po::store(po::parse_command_line(ac, av, desc), vm);
  po::notify(vm);    

  init_logging(verbose);

  if (vm.count("help")) {
    std::cout << desc << "\n";
    return 1;
  }

  generate_data(file_name, file_size*1024*1024);
  return 0;
}
