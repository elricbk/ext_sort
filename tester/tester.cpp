#include <vector>
#include <fstream>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/program_options.hpp>
#include <boost/throw_exception.hpp>
#include <boost/exception/all.hpp>

#include "common/test.h"
#include "common/record_info.h"

std::vector<record_info_t> records_from_file(const std::string& fname)
{
  std::ifstream is(fname.c_str(), std::ifstream::binary);
  std::vector<record_info_t> result;
  while (is.good())
  {
    test cur_test;
    size_t offset = is.tellg();
    is.read(reinterpret_cast<char*>(&cur_test), sizeof(cur_test));
    std::streamsize bytes_read = is.gcount();
    if (bytes_read != sizeof(cur_test))
      BOOST_THROW_EXCEPTION(std::runtime_error("Unable to read test struct from input file"));
    is.ignore(cur_test.size);
    result.push_back(record_info_t(cur_test, offset));
  }
  return result;
}

bool keys_are_equal(const record_info_t& first, const record_info_t& second)
{
  return std::memcmp(first.key, second.key, 64) == 0;
}

bool test_files(const std::string& original, const std::string& sorted)
{
  std::vector<record_info_t> rec_orig = records_from_file(original);
  std::vector<record_info_t> rec_sort = records_from_file(sorted);
  if (rec_orig.empty()) {
    std::cout << "No records found in original file\n";
    return false;
  }
  if (rec_sort.empty()) {
    std::cout << "No records found in sorted file\n";
    return false;
  }
  
  for (size_t i = 0; i < rec_sort.size() - 1; ++i) {
    if (!(rec_sort[i] < rec_sort[i + 1])) {
      std::cout << "Not sorted record found at index " << i << std::endl;
      return false;
    }
  }
  BOOST_FOREACH(const record_info_t& rec, rec_sort) {
    if (std::find_if(rec_orig.begin(), rec_orig.end(), boost::bind(keys_are_equal, rec, _1)) == rec_orig.end()) {
      std::cout << "Sorted key not found in original keys\n";
      return false;
    }
  }
  return true;
}

int main(int ac, const char *av[])
{
  namespace po = boost::program_options;

  std::string original;
  std::string sorted;
  bool verbose = false;

  po::options_description desc("Options for tester");
  desc.add_options()
    ("help,h", "produce help message")
    ("original,o", po::value<std::string>(&original), "set original file name")
    ("sorted,s", po::value<std::string>(&sorted), "set sorted (result) file name")
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

  try {
    if  (!test_files(original, sorted))
      return 2;
  } catch (const boost::exception& e) {
    std::cout << "Exception while comparing files: " << boost::diagnostic_information(e);
    return 3;
  } catch (const std::exception& e) {
    std::cout << "Exception while comparing files: " << e.what() << std::endl;
    return 3;
  } catch (...) {
    std::cout << "Unknown exception while comapring files\n";
    return 3;
  }

  return 0;
}
