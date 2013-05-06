#pragma once

#include <boost/timer/timer.hpp>

#include "log4cpp/Category.hh"

// Вспомогательный класс для логирования времени выполнения в log4cpp::Category
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
