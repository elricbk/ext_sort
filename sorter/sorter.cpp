#include <iostream>
#include <fstream>

#include <boost/assert.hpp>
#include <boost/program_options.hpp>
#include <boost/exception/all.hpp>

#include "log4cpp/Category.hh"
#include "log4cpp/Appender.hh"
#include "log4cpp/FileAppender.hh"
#include "log4cpp/OstreamAppender.hh"
#include "log4cpp/Layout.hh"
#include "log4cpp/PatternLayout.hh"
#include "log4cpp/Priority.hh"

#include "common/test.h"
#include "info_container.h"
#include "file_merger.h"

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

void read_data(const std::string& fname, size_t ram_size)
{
  log4cpp::Category& logger = log4cpp::Category::getRoot();
  logger.debug("Starting reading of data");
  std::ifstream is(fname.c_str(), std::ifstream::binary);
  file_merger_t merger(".");
  {
    info_container_t infos(ram_size);
    while (is.good()) {
      test cur;
      uint64_t offset = is.tellg();
      is.read(reinterpret_cast<char*>(&cur), sizeof(cur));
      if (is.gcount() != sizeof(cur)) {
        logger.error("Cannot read data from input file");
        return;
      }
      logger.debug("Key: %02x %02x %02x %02x, Size: %u", cur.key[0], cur.key[1], cur.key[2], cur.key[3], cur.size);
      is.ignore(cur.size);
      infos.add(cur, offset);
      if (infos.is_full()) {
        infos.log_records("Before sort");
        infos.sort();
        infos.log_records("After sort");
        infos.dump_to_file(merger.next_file());
      }
    }
    if (!infos.is_empty()) {
      infos.log_records("Before sort");
      infos.sort();
      infos.log_records("After sort");
      infos.dump_to_file(merger.next_file());
    }
  }
  merger.merge_files(fname, ram_size);

  logger.debug("Data read");
}

int main(int ac, const char *av[])
{
  namespace po = boost::program_options;

  size_t ram_size;
  std::string file_name;
  bool verbose = true;
  bool randomize;

  po::options_description desc("Options for data sorter");
  desc.add_options()
    ("help,h", "produce help message")
    ("size,s", po::value<size_t>(&ram_size)->default_value(1024), "set memory size in Mb to use for sorting")
    ("input,i", po::value<std::string>(&file_name)->default_value("test.dat"), "set input file name")
    ("verbose,v", po::value<bool>(&verbose)->zero_tokens()->default_value(false), "verbose logging")
    ;

  po::variables_map vm;
  try {
    po::store(po::parse_command_line(ac, av, desc), vm);
    po::notify(vm);
  } catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
    std::cout << desc << "\n";
    return 1;
  }

  if (vm.count("help")) {
    std::cout << desc << "\n";
    return 1;
  }

  init_logging(verbose);
  log4cpp::Category& logger = log4cpp::Category::getRoot();
  logger.info("Starting application...");

  try {
    read_data(file_name, ram_size*1024*1024);
  } catch (const boost::exception& e) {
    logger.critStream() << "Error while reading data: " << boost::diagnostic_information(e);
  } catch (const std::exception& e) {
    logger.crit("Error while reading data: %s", e.what());
  } catch (...) {
    logger.crit("Unknown exception while reading data");
  }

  return 0;
}
