#include <fstream>

#include "log4cpp/Category.hh"

#include "common/test.h"
#include "info_container.h"
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
      info_container_t infos(m_ram_size);
      while (is.good()) {
        test cur;
        uint64_t offset = is.tellg();
        is.read(reinterpret_cast<char*>(&cur), sizeof(cur));
        if (is.gcount() != sizeof(cur)) {
          m_logger.error("Cannot read data from input file");
          return;
        }
        m_logger.debug("Key: %02x %02x %02x %02x, Size: %u", cur.key[0], cur.key[1], cur.key[2], cur.key[3], cur.size);
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
    merger.merge_files(m_infile, m_outfile, m_ram_size);

    m_logger.debug("Data read");
  }

private:
  log4cpp::Category& m_logger;
  std::string m_infile;
  std::string m_outfile;
  size_t m_ram_size;
};
