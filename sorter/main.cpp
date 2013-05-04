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

#include "sorter.hpp"

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

  size_t ram_size;
  std::string infile;
  std::string outfile;
  bool verbose;

  po::options_description desc("Options for data sorter");
  desc.add_options()
    ("help,h", "produce help message")
    ("size,s", po::value<size_t>(&ram_size)->default_value(1024), "set memory size in Mb to use for sorting")
    ("input,i", po::value<std::string>(&infile)->required(), "set input file name")
    ("output,i", po::value<std::string>(&outfile)->required(), "set output file name")
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
    sorter_t sorter(infile, outfile, ram_size*1024*1024);
    sorter.sort_data();
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
