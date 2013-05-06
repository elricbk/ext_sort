#include <fstream>

#include <boost/timer/timer.hpp>
#include <boost/mem_fn.hpp>
#include <boost/bind.hpp>

#include "log4cpp/Category.hh"

#include "common/test.h"
#include "info_container.h"
#include "input_buffer.h"
#include "file_merger.h"

class auto_timer_t
{
public:
  auto_timer_t(log4cpp::Category& logger, const std::string& caption)
    : m_logger(logger)
    , m_caption(caption) {}

  ~auto_timer_t()
  {
    m_logger.noticeStream() << m_caption << ":"
      << m_timer.format(boost::timer::default_places, " %ws wall, %us user + %ss system = %ts CPU (%p%)");
  }

private:
  log4cpp::Category& m_logger;
  std::string m_caption;
  boost::timer::cpu_timer m_timer;
};

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
          auto_timer_t t(m_logger, "Loading data");
          fi.buffer().load_data();
        }
        fi.buffer().get_pointers(infos.pointers());
        {
          auto_timer_t t(m_logger, "Logging records");
          infos.log_records("Before sort");
        }
        {
          auto_timer_t t(m_logger, "Sorting");
          infos.sort();
        }
        {
          auto_timer_t t(m_logger, "Logging records");
          infos.log_records("After sort");
        }
        {
          auto_timer_t t(m_logger, "Dumping to tmp file");
          infos.dump_to_file(merger.next_file());
        }
      }
    }
    {
      auto_timer_t t(m_logger, "Merging files");
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
