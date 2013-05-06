#define BOOST_TEST_MODULE InputBuffer
#define BOOST_TEST_DYN_LINK

#include <sstream>

#include <boost/test/unit_test.hpp>

#include "log4cpp/Category.hh"
#include "log4cpp/Appender.hh"
#include "log4cpp/FileAppender.hh"
#include "log4cpp/PatternLayout.hh"
#include "log4cpp/Priority.hh"

#include "common/record.hpp"
#include "sorter/input_buffer.hpp"

struct LoggingSetup
{
  LoggingSetup()
  {
    log4cpp::PatternLayout* layout = new log4cpp::PatternLayout();
    layout->setConversionPattern("%d [%p] %m%n");
    log4cpp::Appender *appender = new log4cpp::FileAppender("default", "test_input_buffer.log");
    appender->setLayout(layout);

    log4cpp::Category& root = log4cpp::Category::getRoot();
    root.setPriority(log4cpp::Priority::DEBUG);
    root.addAppender(appender);
  }
};

BOOST_GLOBAL_FIXTURE(LoggingSetup)

std::ostream& operator<<(std::ostream& os, const record_t& rec)
{
  BOOST_ASSERT(rec.size <= 1024);
  static char buf[1024] = { 0 };
  os.write(reinterpret_cast<const char*>(&rec), sizeof(rec));
  os.write(buf, rec.size);
  return os;
}

bool operator==(const record_t& left, const record_t& right)
{
  return (std::memcmp(&left, &right, sizeof(record_t)) == 0);
}

BOOST_AUTO_TEST_SUITE(InputBuffer)

  BOOST_AUTO_TEST_CASE(construction) {
    log4cpp::Category::getRoot().notice("=== construction ===");
    std::stringstream ss;
    input_buffer_t ib(ss, 1024);
  }

  BOOST_AUTO_TEST_CASE(single_read) {
    log4cpp::Category::getRoot().notice("=== single_read ===");
    std::stringstream ss;
    input_buffer_t ib(ss, 1024);
    record_t rec = make_record(42, 24, 10);
    ss << rec;
    ib.load_data();
    BOOST_REQUIRE(ib.has_cached_data());
    BOOST_CHECK_EQUAL(*ib.peek(), rec);
  }

  BOOST_AUTO_TEST_CASE(pop) {
    log4cpp::Category::getRoot().notice("=== pop ===");
    std::stringstream ss;
    input_buffer_t ib(ss, 1024);
    record_t rec = make_record(42, 24, 10);
    ss << rec;
    ib.load_data();
    BOOST_REQUIRE(ib.has_cached_data());
    ib.pop();
    BOOST_REQUIRE(!ib.has_cached_data());
  }

  BOOST_AUTO_TEST_CASE(multiple_reads) {
    log4cpp::Category::getRoot().notice("=== multiple_reads ===");
    std::stringstream ss;
    input_buffer_t ib(ss, 200);
    record_t rec = make_record(42, 24, 10);
    record_t rec2 = make_record(31, 45, 3);

    ss << rec << rec << rec2;

    ib.load_data();

    BOOST_REQUIRE(ib.has_cached_data());
    BOOST_CHECK_EQUAL(*ib.peek(), rec);

    while (ib.has_cached_data())
      ib.pop();
    ib.load_data();

    BOOST_REQUIRE(ib.has_cached_data());
    BOOST_CHECK_EQUAL(*ib.peek(), rec2);
  }


  BOOST_AUTO_TEST_CASE(get_pointers_ON_no_data_RETURNS_no_pointers) {
    log4cpp::Category::getRoot().notice("=== get_pointers_ON_no_data_RETURNS_no_pointers ===");
    std::vector<record_t*> ptrs;
    ptrs.push_back(NULL);
    std::stringstream ss;
    input_buffer_t ib(ss, 200);
    ib.get_pointers(&ptrs);
    BOOST_CHECK(ptrs.empty());
  }

  BOOST_AUTO_TEST_CASE(get_pointers_ON_multiple_data_RETURNS_correct_pointers) {
    log4cpp::Category::getRoot().notice("=== get_pointers_ON_multiple_data_RETURNS_correct_pointers ===");
    std::vector<record_t*> ptrs;
    ptrs.push_back(NULL);

    std::stringstream ss;
    record_t rec = make_record(42, 24, 10);
    record_t rec2 = make_record(31, 45, 10);
    ss << rec << rec2 << rec;

    input_buffer_t ib(ss, 200);
    ib.load_data();
    ib.get_pointers(&ptrs);

    BOOST_CHECK_EQUAL(ptrs.size(), 2);
    BOOST_CHECK_EQUAL(*ptrs[0], rec);
    BOOST_CHECK_EQUAL(*ptrs[1], rec2);
  }

BOOST_AUTO_TEST_SUITE_END()
