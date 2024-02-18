
#define NDB_NOLOG

#include "../useful/TestCommon.h"
#include <core/ts/OrderedSeries.h>
#include <core/ts/Series.h>
#include <core/LogFormatter.h>
#include <string_view>
#include <sstream>


using namespace nemesis::test;
using namespace nemesis::core::ts;

static plog::ColorConsoleAppender<NdbFormatter> consoleAppender;


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

    ASSERT_EQ(s.add(query.at("TS_ADD")).status, TsStatus::Ok);
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

    ASSERT_EQ(s.add(query.at("TS_ADD")).status, TsStatus::Ok);
  }
};


TEST_F(TsSeriesTest, Create)
{
  Series s;

  ASSERT_EQ(s.create("os1").status, TsStatus::Ok);
  ASSERT_TRUE(s.containsSeries("os1"));

  ASSERT_EQ(s.create("os1").status, TsStatus::SeriesExists);
}


TEST_F(TsSeriesTest, NotExist)
{
  Series s;

  // single series
  {
    const auto query = njson::parse(R"( { "TS_GET":{"ts":["os1"], "rng":[1,5]} } )");
    auto res = s.get(query.at("TS_GET"));

    //ASSERT_EQ(res.status, TsStatus::SeriesNotExist);
    ASSERT_TRUE(res.rsp.contains("os1"));
    ASSERT_TRUE(res.rsp["os1"].contains("st"));
    ASSERT_TRUE(res.rsp["os1"].contains("t"));
    ASSERT_TRUE(res.rsp["os1"].contains("v"));

    ASSERT_TRUE(res.rsp["os1"]["t"].empty());
    ASSERT_TRUE(res.rsp["os1"]["v"].empty());
  }
  
}


TEST_F(TsSeriesTest, GetSimple)
{
  Series s;

  ASSERT_EQ(s.create("os1").status, TsStatus::Ok);

  addSimpleData("os1", s);
  
  njson rspEverything;

  {
    const auto query = njson::parse(R"( { "TS_GET":{"ts":["os1"], "rng":[1,5]} } )");
    auto res = s.get(query.at("TS_GET"));
    
    //ASSERT_EQ(res.status, TsStatus::Ok);
    ASSERT_TRUE(res.rsp.contains("os1"));
    ASSERT_TRUE(res.rsp["os1"].contains("t"));
    ASSERT_TRUE(res.rsp["os1"].contains("v"));
    ASSERT_TRUE(res.rsp["os1"].contains("st"));

    ASSERT_EQ(res.rsp["os1"]["t"].size(), 4);
    ASSERT_EQ(res.rsp["os1"]["t"].size(), res.rsp["os1"]["v"].size());
    
    rspEverything = res.rsp;
  }

  // same as before but no end set
  {
    const auto query1 = njson::parse(R"( { "TS_GET":{"ts":["os1"], "rng":[1]} } )");
    const auto res = s.get(query1.at("TS_GET"));
    
    ASSERT_EQ(res.status, TsStatus::Ok);
    ASSERT_EQ(res.rsp, rspEverything);
  }
}


TEST_F(TsSeriesTest, GetComplex)
{
  Series s;

  ASSERT_EQ(s.create("os1").status, TsStatus::Ok);

  addComplexData("os1", s);
  
  {
    const auto query = njson::parse(R"( { "TS_GET":{"ts":["os1"], "rng":[10,11]} } )");
    const auto res = s.get(query.at("TS_GET"));
    
    //ASSERT_EQ(res.status, TsStatus::Ok);
    ASSERT_TRUE(res.rsp.contains("os1"));
    ASSERT_TRUE(res.rsp["os1"].contains("t"));
    ASSERT_TRUE(res.rsp["os1"].contains("v"));

    ASSERT_EQ(res.rsp["os1"]["t"].size(), 2);
    ASSERT_EQ(res.rsp["os1"]["t"].size(), res.rsp["os1"]["v"].size());    
  }

  {
    const auto query = njson::parse(R"( { "TS_GET":{"ts":["os1"], "rng":[12]} } )");
    const auto res = s.get(query.at("TS_GET"));
    
    //ASSERT_EQ(res.status, TsStatus::Ok);
    ASSERT_TRUE(res.rsp.contains("os1"));
    ASSERT_TRUE(res.rsp["os1"].contains("t"));
    ASSERT_TRUE(res.rsp["os1"].contains("v"));

    ASSERT_EQ(res.rsp["os1"]["t"].size(), 1);
    ASSERT_EQ(res.rsp["os1"]["t"].size(), res.rsp["os1"]["v"].size());
  }
}


/// Multiple series, same range for each
TEST_F(TsSeriesTest, GetMultipleSeries)
{
  Series s;

  ASSERT_EQ(s.create("os1").status, TsStatus::Ok);
  ASSERT_EQ(s.create("os2").status, TsStatus::Ok);

  addSimpleData("os1", s);
  addSimpleData("os2", s);
  
  const auto query = njson::parse(R"( { "TS_GET":{"ts":["os1","os2"], "rng":[1,5]} } )");
  const auto res = s.get(query.at("TS_GET"));

  //ASSERT_EQ(res.status, TsStatus::Ok);

  ASSERT_TRUE(res.rsp.contains("os1"));
  ASSERT_TRUE(res.rsp.contains("os2"));
  
  ASSERT_TRUE(res.rsp["os1"].contains("t"));
  ASSERT_TRUE(res.rsp["os1"].contains("v"));
  ASSERT_TRUE(res.rsp["os2"].contains("t"));
  ASSERT_TRUE(res.rsp["os2"].contains("v"));

  ASSERT_EQ(res.rsp["os1"]["t"].size(), 4);
  ASSERT_EQ(res.rsp["os1"]["t"].size(), res.rsp["os1"]["v"].size());
  ASSERT_EQ(res.rsp["os2"]["t"].size(), 4);
  ASSERT_EQ(res.rsp["os2"]["t"].size(), res.rsp["os2"]["v"].size());
}


/// Multiple series, different range for each
TEST_F(TsSeriesTest, GetMultipleRanges)
{
  Series s;

  ASSERT_EQ(s.create("os1").status, TsStatus::Ok);
  ASSERT_EQ(s.create("os2").status, TsStatus::Ok);

  addSimpleData("os1", s);
  addSimpleData("os2", s);

  const auto query = njson::parse(R"( { "TS_GET_MULTI":{ "os1": {"rng":[1,3]}, "os2": {"rng":[1,5]}} } )");
  const auto res = s.getMultipleRanges(query.at("TS_GET_MULTI"));

  ASSERT_TRUE(res.rsp.contains("os1"));
  ASSERT_TRUE(res.rsp.contains("os2"));

  ASSERT_TRUE(res.rsp["os1"].contains("t"));
  ASSERT_TRUE(res.rsp["os1"].contains("v"));
  ASSERT_TRUE(res.rsp["os2"].contains("t"));
  ASSERT_TRUE(res.rsp["os2"].contains("v"));

  ASSERT_EQ(res.rsp["os1"]["t"].size(), 3);
  ASSERT_EQ(res.rsp["os1"]["t"].size(), res.rsp["os1"]["v"].size());
  ASSERT_EQ(res.rsp["os2"]["t"].size(), 4);
  ASSERT_EQ(res.rsp["os2"]["t"].size(), res.rsp["os2"]["v"].size());
}


int main (int argc, char ** argv)
{
	testing::InitGoogleTest(&argc, argv);	

	return RUN_ALL_TESTS();	
}
