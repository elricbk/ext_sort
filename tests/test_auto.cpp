#define BOOST_TEST_MODULE Autotests
#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>

#include "log4cpp/Category.hh"
#include "log4cpp/Appender.hh"
#include "log4cpp/FileAppender.hh"
#include "log4cpp/PatternLayout.hh"
#include "log4cpp/Priority.hh"

#include "sorter/sorter.hpp"
#include "generator/generator.hpp"
#include "file_comparer.hpp"

struct LoggingSetup
{
  LoggingSetup()
  {
    log4cpp::PatternLayout* layout = new log4cpp::PatternLayout();
    layout->setConversionPattern("%d [%p] %m%n");
    log4cpp::Appender *appender = new log4cpp::FileAppender("default", "test_auto.log");
    appender->setLayout(layout);

    log4cpp::Category& root = log4cpp::Category::getRoot();
    root.setPriority(log4cpp::Priority::DEBUG);
    root.addAppender(appender);
  }
};

BOOST_GLOBAL_FIXTURE(LoggingSetup)

BOOST_AUTO_TEST_SUITE(SimpleScnearios)

  BOOST_AUTO_TEST_CASE(single_temp_file) {
    log4cpp::Category::getRoot().notice("===  single_temp_file ===");
    file_comparer_t comparer;

    generator_t generator(comparer.fname_orig(), 1024, 50);
    generator.generate_data();

    sorter_t sorter(comparer.fname_orig(), comparer.fname_sort(), 20480);
    sorter.sort_data();

    BOOST_CHECK(comparer.file_is_sorted());
    BOOST_CHECK(comparer.records_are_equal());
  }

  BOOST_AUTO_TEST_CASE(multiple_temp_files) {
    log4cpp::Category::getRoot().notice("===  multiple_temp_files ===");
    file_comparer_t comparer;

    generator_t generator(comparer.fname_orig(), 20480, 50);
    generator.generate_data();

    sorter_t sorter(comparer.fname_orig(), comparer.fname_sort(), 4096);
    sorter.sort_data();

    BOOST_CHECK(comparer.file_is_sorted());
    BOOST_CHECK(comparer.records_are_equal());
  }

  BOOST_AUTO_TEST_CASE(sparse_file) {
    log4cpp::Category::getRoot().notice("===  sparse_file ===");
    file_comparer_t comparer;

    generator_t generator(comparer.fname_orig(), 16*1024*1024, 2*1024*1024);
    generator.generate_data();

    sorter_t sorter(comparer.fname_orig(), comparer.fname_sort(), 8*1024*1024);
    sorter.sort_data();

    BOOST_CHECK(comparer.file_is_sorted());
    BOOST_CHECK(comparer.records_are_equal());
  }

  BOOST_AUTO_TEST_CASE(huge_mem_for_sort) {
    // проверяет нормальную работу сортировки для большого объёма выделенной памяти
    // т.к. были проблемы при использовании STL-потоков
    log4cpp::Category::getRoot().notice("=== huge_mem_for_sort ===");
    file_comparer_t comparer;

    generator_t generator(comparer.fname_orig(), 20480, 50);
    generator.generate_data();

    sorter_t sorter(comparer.fname_orig(), comparer.fname_sort(), 4*1024*1024*1024L);
    sorter.sort_data();

    BOOST_CHECK(comparer.file_is_sorted());
    BOOST_CHECK(comparer.records_are_equal());
  }
BOOST_AUTO_TEST_SUITE_END()
