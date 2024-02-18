#ifndef NDB_TEST_TSSERIESTEST_H
#define NDB_TEST_TSSERIESTEST_H


#define NDB_UNIT_TEST 1


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


class TsSeriesTest : public ::testing::Test
{

protected:
  // TODO these should come from core header
  const std::string GetRspCmd           = "TS_GET_RSP";
  const std::string GetMultiRspCmd      = "TS_GET_MULTI_RSP";
  const std::string CreateRspCmd        = "TS_CREATE_RSP";
  const std::string AddRspCmd           = "TS_ADD_RSP";


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

  void addSimpleData(const SeriesName& name, Series& s) const
  {
    std::stringstream ss;
    ss << R"(
              {
                "ts":")" << name << R"(",
                "t":[1,2,3,5],
                "v":["1","2","3","5"]
              }
            )";


    njson query = njson::parse(ss.str());

    ASSERT_EQ(s.add(query, AddRspCmd).status, TsRequestStatus::Ok);
  }


  void addComplexData(const SeriesName& name, Series& s) const
  {
    std::stringstream ss;
    ss << R"(
              {
                "ts":")" << name << R"(",
                "t":[10,11,12],
                "v":[{"x":"x10", "y":"y10"},{"x":"x11", "y":"y11"},{"x":"x12", "y":"y12"}]
              }
            )";


    njson query = njson::parse(ss.str());

    ASSERT_EQ(s.add(query, AddRspCmd).status, TsRequestStatus::Ok);
  }


  void createFilterTestData(Series& s) const
  {
    ASSERT_EQ(s.create("os1", CreateRspCmd).status, TsRequestStatus::Ok);
    ASSERT_EQ(s.create("os2", CreateRspCmd).status, TsRequestStatus::Ok);
    
    std::stringstream ss;
    ss << R"(
              {
                "ts":"os1",
                "t":[1,2,3,4,5],
                "v":[{"temp":10},{"temp":5},{"temp":8},{"temp":4},{"temp":2}]
              }
            )";

    njson data1 = njson::parse(ss.str());
    
    njson data2 = data1;
    data2["ts"] = "os2";
    data2["v"] = njson::parse(R"([{"temp":15},{"temp":5},{"temp":7},{"temp":3},{"temp":2}])");

    ASSERT_EQ(s.add(data1, AddRspCmd).status, TsRequestStatus::Ok);
    ASSERT_EQ(s.add(data2, AddRspCmd).status, TsRequestStatus::Ok);
  }
};


} // ns test
} // ns nemesis

#endif
