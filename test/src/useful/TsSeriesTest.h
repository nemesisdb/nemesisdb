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
#include <fstream>


namespace nemesis { namespace test {

using namespace nemesis::core;
using namespace nemesis::core::ts;


static inline plog::ColorConsoleAppender<NdbFormatter> consoleAppender;


template<typename R>
struct BasicMeasureDuration
{
  BasicMeasureDuration (std::function<R()> f, const std::string_view msg = "")
  {
    const auto start = chrono::steady_clock::now();

    result = f();

    #ifdef NDEBUG
    PLOGI << msg << ":" << chrono::duration_cast<chrono::microseconds> (chrono::steady_clock::now() - start).count() << '\n';
    #endif
  }

  R result;
};


using MeasureDuration = BasicMeasureDuration<njson>;



class TsSeriesTest : public ::testing::Test
{

protected:
  // TODO these should come from core header
  const std::string GetRspCmd             = "TS_GET_RSP";
  const std::string GetMultiRspCmd        = "TS_GET_MULTI_RSP";
  const std::string CreateRspCmd          = "TS_CREATE_RSP";
  const std::string AddRspCmd             = "TS_ADD_EVT_RSP";
  const std::string CreateIndexRspCmd     = "TS_CREATE_INDEX_RSP";


public:
  TsSeriesTest() = default;

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


  // the file should contain an array at the root level with two elements, both are objects: 
  //  [0] - { "TS_CREATE": { ... } }
  //  [1] - { "TS_ADD": { ... } }
  void createMoreData(const fs::path& queryPath, Series& s, const bool createIndex)
  {
    ASSERT_TRUE(fs::exists(queryPath));

    std::ifstream stream{queryPath};

    ASSERT_TRUE(stream.good());

    njson json = njson::parse(stream);

    ASSERT_TRUE(json.is_array());
    ASSERT_EQ(json.size(), 2);

    ASSERT_TRUE(json[0].is_object());
    ASSERT_TRUE(json[1].is_object());

    ASSERT_TRUE(json[0].contains("TS_CREATE"));
    ASSERT_TRUE(json[1].contains("TS_ADD_EVT"));

    ASSERT_EQ(s.create(json[0]["TS_CREATE"]["ts"].as_string(), CreateRspCmd).status, TsRequestStatus::Ok);

    if (createIndex)
      ASSERT_EQ(s.createIndex("os1", "temp", CreateIndexRspCmd).status, TsRequestStatus::Ok);

    BasicMeasureDuration<TsRequestStatus> md {[&s, &json, AddRspCmd = AddRspCmd](){ return s.add(std::move(json[1]["TS_ADD_EVT"]), AddRspCmd).status;}, "TS_ADD"};
    ASSERT_EQ(md.result, TsRequestStatus::Ok);
  }
};


} // ns test
} // ns nemesis

#endif
