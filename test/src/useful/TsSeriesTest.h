#ifndef NDB_TEST_TSSERIESTEST_H
#define NDB_TEST_TSSERIESTEST_H

#include <core/NemesisCommon.h>
#include <core/LogFormatter.h>
#include <core/ts/TsCommon.h>
#include <core/ts/Series.h>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <sstream>


namespace nemesis { namespace test {

using namespace nemesis::core;
using namespace nemesis::core::ts;


static inline plog::ColorConsoleAppender<NdbFormatter> consoleAppender;


class TsSeriesTest  : public ::testing::Test
{
public:
  TsSeriesTest()
  {

  }

	virtual ~TsSeriesTest() = default;


  virtual void SetUp() override
	{
    initLogger (consoleAppender);
	}


	void TearDown() override
	{
		
	}


protected:

  void addSimpleData(const SeriesName& name, Series& s)
  {
    std::stringstream ss;
    ss << R"(
              {
                "TS_ADD":
                {
                  "ts":")" << name << R"(",
                  "t":[1,2,3,5],
                  "v":["1","2","3","5"]
                }
              }
            )";


    njson query = njson::parse(ss.str());

    ASSERT_EQ(s.add(query.at("TS_ADD")).status, TsRequestStatus::Ok);
  }


  void addComplexData(const SeriesName& name, Series& s)
  {
    std::stringstream ss;
    ss << R"(
              {
                "TS_ADD":
                {
                  "ts":")" << name << R"(",
                  "t":[10,11,12],
                  "v":[{"x":"x10", "y":"y10"},{"x":"x11", "y":"y11"},{"x":"x12", "y":"y12"}]
                }
              }
            )";


    njson query = njson::parse(ss.str());

    ASSERT_EQ(s.add(query.at("TS_ADD")).status, TsRequestStatus::Ok);
  }
};


} // ns test
} // ns nemesis

#endif
