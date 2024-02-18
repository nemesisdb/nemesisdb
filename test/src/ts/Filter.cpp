#define NDB_NOLOG

#include "../useful/TsSeriesTest.h"
#include <core/ts/OrderedSeries.h>
#include <string_view>
#include <sstream>


using namespace nemesis::test;
using namespace nemesis::core;
using namespace nemesis::core::ts;



TEST_F(TsSeriesTest, Get)
{
  Series s;
  createFilterTestData(s);

  // rng: all, where: everything
  {    
    njson q = njson::parse(R"( {"ts":["os1"], "rng":[1,5], "where":"$[?($.temp <= 10)]"})");
    auto r = s.get(q, GetRspCmd);

    ASSERT_EQ(r.status, TsRequestStatus::Ok);
    ASSERT_TRUE(r.rsp[GetRspCmd]["os1"].contains("t"));
    ASSERT_TRUE(r.rsp[GetRspCmd]["os1"].contains("v"));

    ASSERT_EQ(r.rsp[GetRspCmd]["os1"]["t"].size(), 5);
    ASSERT_EQ(r.rsp[GetRspCmd]["os1"]["t"].size(), r.rsp[GetRspCmd]["os1"]["v"].size());

    ASSERT_EQ(r.rsp[GetRspCmd]["os1"]["t"], njson::parse(R"([1,2,3,4,5])"));
  }


  // rng: middle, where: everything
  {    
    njson q = njson::parse(R"( {"ts":["os1"], "rng":[2,4], "where":"$[?($.temp <= 8)]"})");
    auto r = s.get(q, GetRspCmd);

    ASSERT_EQ(r.status, TsRequestStatus::Ok);
    ASSERT_TRUE(r.rsp[GetRspCmd]["os1"].contains("t"));
    ASSERT_TRUE(r.rsp[GetRspCmd]["os1"].contains("v"));

    ASSERT_EQ(r.rsp[GetRspCmd]["os1"]["t"].size(), 3);
    ASSERT_EQ(r.rsp[GetRspCmd]["os1"]["t"].size(), r.rsp[GetRspCmd]["os1"]["v"].size());

    ASSERT_EQ(r.rsp[GetRspCmd]["os1"]["t"], njson::parse(R"([2,3,4])"));
  }


  // rng: all, where: two conditions
  {    
    njson q = njson::parse(R"( {"ts":["os1"], "rng":[1,5], "where":"$[?($.temp >= 4 && $.temp <= 8)]"})");
    auto r = s.get(q, GetRspCmd);

    ASSERT_EQ(r.status, TsRequestStatus::Ok);
    ASSERT_TRUE(r.rsp[GetRspCmd]["os1"].contains("t"));
    ASSERT_TRUE(r.rsp[GetRspCmd]["os1"].contains("v"));

    ASSERT_EQ(r.rsp[GetRspCmd]["os1"]["t"].size(), 3);
    ASSERT_EQ(r.rsp[GetRspCmd]["os1"]["t"].size(), r.rsp[GetRspCmd]["os1"]["v"].size());

    ASSERT_EQ(r.rsp[GetRspCmd]["os1"]["t"], njson::parse(R"([2,3,4])"));
  } 
}


TEST_F(TsSeriesTest, GetMultiple)
{
  Series s;
  createFilterTestData(s);

  // rng: os1: all, os2: all, where: everything
  {    
    njson q = njson::parse(R"(  {
                                  "os1":{ "rng":[1,5], "where":"$[?($.temp <= 10)]"},
                                  "os2":{ "rng":[1,5], "where":"$[?($.temp <= 15)]"}
                                })");

    auto r = s.getMultipleRanges(q, GetMultiRspCmd);

    ASSERT_EQ(r.status, TsRequestStatus::Ok);

    // os1
    ASSERT_TRUE(r.rsp[GetMultiRspCmd]["os1"].contains("t"));
    ASSERT_TRUE(r.rsp[GetMultiRspCmd]["os1"].contains("v"));

    ASSERT_EQ(r.rsp[GetMultiRspCmd]["os1"]["t"].size(), 5);
    ASSERT_EQ(r.rsp[GetMultiRspCmd]["os1"]["t"].size(), r.rsp[GetMultiRspCmd]["os1"]["v"].size());

    ASSERT_EQ(r.rsp[GetMultiRspCmd]["os1"]["t"], njson::parse(R"([1,2,3,4,5])"));

    // os2
    ASSERT_TRUE(r.rsp[GetMultiRspCmd]["os2"].contains("t"));
    ASSERT_TRUE(r.rsp[GetMultiRspCmd]["os2"].contains("v"));

    ASSERT_EQ(r.rsp[GetMultiRspCmd]["os2"]["t"].size(), 5);
    ASSERT_EQ(r.rsp[GetMultiRspCmd]["os2"]["t"].size(), r.rsp[GetMultiRspCmd]["os2"]["v"].size());

    ASSERT_EQ(r.rsp[GetMultiRspCmd]["os2"]["t"], njson::parse(R"([1,2,3,4,5])"));
  }


  // os1: range: all, where: everything
  // os2: range: mid, where: everything
  {    
    njson q = njson::parse(R"(  {
                                  "os1":{ "rng":[1,5], "where":"$[?($.temp <= 10)]"},
                                  "os2":{ "rng":[2,4], "where":"$[?($.temp <= 7)]"}
                                })");

    auto r = s.getMultipleRanges(q, GetMultiRspCmd);

    ASSERT_EQ(r.status, TsRequestStatus::Ok);

    // os1
    ASSERT_TRUE(r.rsp[GetMultiRspCmd]["os1"].contains("t"));
    ASSERT_TRUE(r.rsp[GetMultiRspCmd]["os1"].contains("v"));

    ASSERT_EQ(r.rsp[GetMultiRspCmd]["os1"]["t"].size(), 5);
    ASSERT_EQ(r.rsp[GetMultiRspCmd]["os1"]["t"].size(), r.rsp[GetMultiRspCmd]["os1"]["v"].size());

    ASSERT_EQ(r.rsp[GetMultiRspCmd]["os1"]["t"], njson::parse(R"([1,2,3,4,5])"));

    // os2
    ASSERT_TRUE(r.rsp[GetMultiRspCmd]["os2"].contains("t"));
    ASSERT_TRUE(r.rsp[GetMultiRspCmd]["os2"].contains("v"));

    ASSERT_EQ(r.rsp[GetMultiRspCmd]["os2"]["t"].size(), 3);
    ASSERT_EQ(r.rsp[GetMultiRspCmd]["os2"]["t"].size(), r.rsp[GetMultiRspCmd]["os2"]["v"].size());

    ASSERT_EQ(r.rsp[GetMultiRspCmd]["os2"]["t"], njson::parse(R"([2,3,4])"));
  }
}



int main (int argc, char ** argv)
{
	testing::InitGoogleTest(&argc, argv);	

	return RUN_ALL_TESTS();	
}
