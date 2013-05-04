#include <iostream>
#include <boost/program_options.hpp>
#include <boost/exception/all.hpp>

#include "log4cpp/Category.hh"
#include "log4cpp/Appender.hh"
#include "log4cpp/FileAppender.hh"
#include "log4cpp/OstreamAppender.hh"
#include "log4cpp/Layout.hh"
#include "log4cpp/PatternLayout.hh"
#include "log4cpp/Priority.hh"

#include "generator.hpp"

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
  size_t data_size;
  std::string file_name;
  bool verbose;
  bool randomize;

  po::options_description desc("Options for test data generator");
  desc.add_options()
    ("help,h", "produce help message")
    ("size,s", po::value<size_t>(&file_size)->default_value(1024), "set approximate output file size in Mb, generation will stop when this size is exceeded")
    ("datasize,d", po::value<size_t>(&data_size)->default_value(1), "set maximum size of data for each record in Kb")
    ("output,o", po::value<std::string>(&file_name)->default_value("test.dat"), "set output file name")
    ("verbose,v", po::value<bool>(&verbose)->zero_tokens()->default_value(false), "verbose logging")
    ("randomize-data", po::value<bool>(&randomize)->default_value(false), "set random seed for data generator")
    ;

  po::variables_map vm;
  po::store(po::parse_command_line(ac, av, desc), vm);
  po::notify(vm);    

  init_logging(verbose);
  log4cpp::Category& logger = log4cpp::Category::getRoot();

  if (vm.count("help")) {
    std::cout << desc << "\n";
    return 1;
  }

  try {
    generator_t generator(file_name, file_size*1024*1024, data_size*1024);
    generator.generate_data();
  } catch (const boost::exception& e) {
    logger.critStream() << "Error while reading data: " << boost::diagnostic_information(e);
    return 2;
  } catch (const std::exception& e) {
    logger.crit("Error while reading data: %s", e.what());
    return 2;
  } catch (...) {
    logger.crit("Unknown exception while reading data");
    return 2;
  }

  return 0;
}
