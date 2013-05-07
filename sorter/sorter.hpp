#include "log4cpp/Category.hh"

#include "common/utils.hpp"
#include "info_container.hpp"
#include "input_buffer.hpp"
#include "file_merger.hpp"

//! Основной класс для внешней сортировки
class sorter_t {
public:
  sorter_t(const std::string& infile, const std::string& outfile, size_t ram_size)
    : m_logger(log4cpp::Category::getRoot())
    , m_infile(infile)
    , m_outfile(outfile)
    , m_ram_size(ram_size) {}

  //! Сортировка данных
  /** Журналирует время выполнения различных шагов сортировки */
  void sort_data()
  {
    m_logger.debug("Starting reading of data");
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
        {
          auto_timer_t t(m_logger, "Building pointers for sort");
          fi.buffer().get_pointers(infos.pointers());
        }
        {
          auto_timer_t t(m_logger, "Sorting");
          infos.sort();
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
