#include <fstream>

#include <boost/timer/timer.hpp>

#include "log4cpp/Category.hh"

#include "common/test.h"
#include "info_container.h"
#include "input_buffer.h"
#include "file_merger.h"

class sorter_t
{
public:
  sorter_t(const std::string& infile, const std::string& outfile, size_t ram_size)
    : m_logger(log4cpp::Category::getRoot())
    , m_infile(infile)
    , m_outfile(outfile)
    , m_ram_size(ram_size) {}

  void sort_data()
  {
    m_logger.debug("Starting reading of data");
    std::ifstream is(m_infile.c_str(), std::ifstream::binary);
    file_merger_t merger(".");
    {
      size_t max_record_count = m_ram_size/(sizeof(record_t) + sizeof(record_t*));
      info_container_t infos(max_record_count);
      input_file_t fi(m_infile, max_record_count*sizeof(record_t));
      while (!fi.buffer().eof()) {
        {
          boost::timer::auto_cpu_timer t("Loading data: %w second(s)\n");
          fi.buffer().load_data();
        }
        fi.buffer().get_pointers(infos.pointers());
        {
          boost::timer::auto_cpu_timer t("Logging records: %w second(s)\n");
          infos.log_records("Before sort");
        }
        {
          boost::timer::auto_cpu_timer t("Sorting data: %w second(s)\n");
          infos.sort();
        }
        {
          boost::timer::auto_cpu_timer t("Logging records: %w second(s)\n");
          infos.log_records("After sort");
        }
        {
          boost::timer::auto_cpu_timer t("Dumping to tmp file: %w second(s)\n");
          infos.dump_to_file(merger.next_file());
        }
        while (fi.buffer().has_cached_data())
          fi.buffer().pop();
      }
    }
    {
      boost::timer::auto_cpu_timer t("Merging data: %w second(s)\n");
      merger.merge_files(m_outfile, m_ram_size);
    }

    m_logger.debug("Data read");
  }

private:
  log4cpp::Category& m_logger;
  std::string m_infile;
  std::string m_outfile;
  size_t m_ram_size;
};
