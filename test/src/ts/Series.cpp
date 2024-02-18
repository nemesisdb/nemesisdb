
#define NDB_NOLOG

#include "../useful/TsSeriesTest.h"
#include <core/ts/OrderedSeries.h>
#include <string_view>
#include <sstream>


using namespace nemesis::test;
using namespace nemesis::core::ts;
using namespace nemesis::core;


TEST_F(TsSeriesTest, Create)
{
  Series s;

  ASSERT_EQ(s.create("os1", CreateRspCmd).status, TsRequestStatus::Ok);
  ASSERT_TRUE(s.hasSeries("os1"));

  ASSERT_EQ(s.create("os1", CreateRspCmd).status, TsRequestStatus::SeriesExists);
}


TEST_F(TsSeriesTest, NotExist)
{
  Series s;

  // single series
  {
    const auto query = njson::parse(R"( {"ts":["os1"], "rng":[1,5]} )");
    auto res = s.get(query, GetRspCmd);

    //ASSERT_EQ(res.status, TsRequestStatus::SeriesNotExist);
    ASSERT_TRUE(res.rsp[GetRspCmd].contains("os1"));
    ASSERT_TRUE(res.rsp[GetRspCmd]["os1"].contains("st"));
    ASSERT_TRUE(res.rsp[GetRspCmd]["os1"].contains("t"));
    ASSERT_TRUE(res.rsp[GetRspCmd]["os1"].contains("v"));

    ASSERT_TRUE(res.rsp[GetRspCmd]["os1"]["t"].empty());
    ASSERT_TRUE(res.rsp[GetRspCmd]["os1"]["v"].empty());
  }
  
}


TEST_F(TsSeriesTest, GetSimple)
{
  Series s;

  ASSERT_EQ(s.create("os1", CreateRspCmd).status, TsRequestStatus::Ok);

  addSimpleData("os1", s);
  
  njson rspEverything;

  {
    const auto query = njson::parse(R"( {"ts":["os1"], "rng":[1,5]} )");
    auto res = s.get(query, GetRspCmd);
    
    //ASSERT_EQ(res.status, TsRequestStatus::Ok);
    ASSERT_TRUE(res.rsp[GetRspCmd].contains("os1"));
    ASSERT_TRUE(res.rsp[GetRspCmd]["os1"].contains("t"));
    ASSERT_TRUE(res.rsp[GetRspCmd]["os1"].contains("v"));
    ASSERT_TRUE(res.rsp[GetRspCmd]["os1"].contains("st"));

    ASSERT_EQ(res.rsp[GetRspCmd]["os1"]["t"].size(), 4);
    ASSERT_EQ(res.rsp[GetRspCmd]["os1"]["t"].size(), res.rsp[GetRspCmd]["os1"]["v"].size());
    
    rspEverything = res.rsp;
  }

  // same as before but no end set
  {
    const auto query1 = njson::parse(R"( {"ts":["os1"], "rng":[1]} )");
    const auto res = s.get(query1, GetRspCmd);
    
    ASSERT_EQ(res.status, TsRequestStatus::Ok);
    ASSERT_EQ(res.rsp, rspEverything);
  }
}


TEST_F(TsSeriesTest, GetComplex)
{
  Series s;

  ASSERT_EQ(s.create("os1", CreateRspCmd).status, TsRequestStatus::Ok);

  addComplexData("os1", s);
  
  {
    const auto query = njson::parse(R"( {"ts":["os1"], "rng":[10,11]} )");
    const auto res = s.get(query, GetRspCmd);
    
    //ASSERT_EQ(res.status, TsRequestStatus::Ok);
    ASSERT_TRUE(res.rsp[GetRspCmd].contains("os1"));
    ASSERT_TRUE(res.rsp[GetRspCmd]["os1"].contains("t"));
    ASSERT_TRUE(res.rsp[GetRspCmd]["os1"].contains("v"));

    ASSERT_EQ(res.rsp[GetRspCmd]["os1"]["t"].size(), 2);
    ASSERT_EQ(res.rsp[GetRspCmd]["os1"]["t"].size(), res.rsp[GetRspCmd]["os1"]["v"].size());    
  }

  {
    const auto query = njson::parse(R"( {"ts":["os1"], "rng":[12]} )");
    const auto res = s.get(query, GetRspCmd);
    
    //ASSERT_EQ(res.status, TsRequestStatus::Ok);
    ASSERT_TRUE(res.rsp[GetRspCmd].contains("os1"));
    ASSERT_TRUE(res.rsp[GetRspCmd]["os1"].contains("t"));
    ASSERT_TRUE(res.rsp[GetRspCmd]["os1"].contains("v"));

    ASSERT_EQ(res.rsp[GetRspCmd]["os1"]["t"].size(), 1);
    ASSERT_EQ(res.rsp[GetRspCmd]["os1"]["t"].size(), res.rsp[GetRspCmd]["os1"]["v"].size());
  }
}


/// Multiple series, same range for each
TEST_F(TsSeriesTest, GetMultipleSeries)
{
  Series s;

  ASSERT_EQ(s.create("os1", CreateRspCmd).status, TsRequestStatus::Ok);
  ASSERT_EQ(s.create("os2", CreateRspCmd).status, TsRequestStatus::Ok);

  addSimpleData("os1", s);
  addSimpleData("os2", s);
  
  const auto query = njson::parse(R"( {"ts":["os1","os2"], "rng":[1,5]} )");
  const auto res = s.get(query, GetRspCmd);

  //ASSERT_EQ(res.status, TsRequestStatus::Ok);

  ASSERT_TRUE(res.rsp[GetRspCmd].contains("os1"));
  ASSERT_TRUE(res.rsp[GetRspCmd].contains("os2"));
  
  ASSERT_TRUE(res.rsp[GetRspCmd]["os1"].contains("t"));
  ASSERT_TRUE(res.rsp[GetRspCmd]["os1"].contains("v"));
  ASSERT_TRUE(res.rsp[GetRspCmd]["os2"].contains("t"));
  ASSERT_TRUE(res.rsp[GetRspCmd]["os2"].contains("v"));

  ASSERT_EQ(res.rsp[GetRspCmd]["os1"]["t"].size(), 4);
  ASSERT_EQ(res.rsp[GetRspCmd]["os1"]["t"].size(), res.rsp[GetRspCmd]["os1"]["v"].size());
  ASSERT_EQ(res.rsp[GetRspCmd]["os2"]["t"].size(), 4);
  ASSERT_EQ(res.rsp[GetRspCmd]["os2"]["t"].size(), res.rsp[GetRspCmd]["os2"]["v"].size());
}


/// Multiple series, different range for each
TEST_F(TsSeriesTest, GetMultipleRanges)
{
  Series s;

  ASSERT_EQ(s.create("os1", CreateRspCmd).status, TsRequestStatus::Ok);
  ASSERT_EQ(s.create("os2", CreateRspCmd).status, TsRequestStatus::Ok);

  addSimpleData("os1", s);
  addSimpleData("os2", s);

  const auto query = njson::parse(R"( { "os1": {"rng":[1,3]}, "os2": {"rng":[1,5]}} )");
  const auto res = s.getMultipleRanges(query, GetMultiRspCmd);

  ASSERT_TRUE(res.rsp[GetMultiRspCmd].contains("os1"));
  ASSERT_TRUE(res.rsp[GetMultiRspCmd].contains("os2"));

  ASSERT_TRUE(res.rsp[GetMultiRspCmd]["os1"].contains("t"));
  ASSERT_TRUE(res.rsp[GetMultiRspCmd]["os1"].contains("v"));
  ASSERT_TRUE(res.rsp[GetMultiRspCmd]["os2"].contains("t"));
  ASSERT_TRUE(res.rsp[GetMultiRspCmd]["os2"].contains("v"));

  ASSERT_EQ(res.rsp[GetMultiRspCmd]["os1"]["t"].size(), 3);
  ASSERT_EQ(res.rsp[GetMultiRspCmd]["os1"]["t"].size(), res.rsp[GetMultiRspCmd]["os1"]["v"].size());
  ASSERT_EQ(res.rsp[GetMultiRspCmd]["os2"]["t"].size(), 4);
  ASSERT_EQ(res.rsp[GetMultiRspCmd]["os2"]["t"].size(), res.rsp[GetMultiRspCmd]["os2"]["v"].size());
}


int main (int argc, char ** argv)
{
	testing::InitGoogleTest(&argc, argv);	

	return RUN_ALL_TESTS();	
}
