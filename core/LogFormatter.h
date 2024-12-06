#ifndef NBD_LOGFORMATTER_H
#define NBD_LOGFORMATTER_H

#include <plog/Record.h>
#include <plog/Util.h>
#include <iomanip>


namespace nemesis {

  template<bool useUtcTime>
  class NdbFormatterImpl
  {
  public:
    static plog::util::nstring header()
    {
      return plog::util::nstring();
    }

    static plog::util::nstring format(const plog::Record& record)
    {
      tm t;
      useUtcTime ? plog::util::gmtime_s(&t, &record.getTime().time) : plog::util::localtime_s(&t, &record.getTime().time);

      plog::util::nostringstream ss;
      ss << t.tm_year + 1900 << "-" << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_mon + 1 << PLOG_NSTR("-") << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_mday << PLOG_NSTR(" ");
      ss << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_hour << PLOG_NSTR(":") << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_min << PLOG_NSTR(":") << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_sec << PLOG_NSTR(".") << std::setfill(PLOG_NSTR('0')) << std::setw(3) << static_cast<int> (record.getTime().millitm) << PLOG_NSTR(" ");
      ss << std::setfill(PLOG_NSTR(' ')) << std::setw(5) << std::left << plog::severityToString(record.getSeverity()) << PLOG_NSTR(" ");
      ss << record.getMessage() << PLOG_NSTR("\n");

      return ss.str();
    }
  };

  class NdbFormatter : public NdbFormatterImpl<false> {};
  class NdbFormatterUtcTime : public NdbFormatterImpl<true> {};

}

#endif
