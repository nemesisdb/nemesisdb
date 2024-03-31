//#define NDB_NOLOG

#include "../useful/TsSeriesTest.h"
#include <core/ts/OrderedSeries.h>
#include <string_view>
#include <sstream>


using namespace nemesis::test;
using namespace nemesis::core;
using namespace nemesis::core::ts;



TEST_F(TsSeriesTest, Test10k_Index)
{
  Series s;
  createMoreData({"../test_data/moredata_10k.json"}, s, true);
  
  
  // >
  {
    auto q = njson::parse(R"(
                              {
                                "ts":"os1",
                                "rng":[4000,6000],
                                "where":
                                {
                                  "temp":
                                  {
                                    ">":0
                                  }
                                }
                              }
                            )");

    MeasureDuration md {[&s, &q, rspName = GetRspCmd]{ return s.get(q, rspName).rsp; }};
  }

  
  // >
  {
    auto q = njson::parse(R"(
                              {
                                "ts":"os1",
                                "rng":[0,10],
                                "where":
                                {
                                  "temp":
                                  {
                                    ">":20
                                  }
                                }
                              }
                            )");

    MeasureDuration md {[&s, &q, rspName = GetRspCmd]{ return s.get(q, rspName).rsp; }};
    ASSERT_EQ(md.result["TS_GET_RSP"]["t"].size(),2);
  }

  
  // >=
  {
    auto q = njson::parse(R"(
                              {
                                "ts":"os1",
                                "rng":[0,10],
                                "where":
                                {
                                  "temp":
                                  {
                                    ">=":20
                                  }
                                }
                              }
                            )");

    MeasureDuration md {[&s, &q, rspName = GetRspCmd]{ return s.get(q, rspName).rsp; }};
    ASSERT_EQ(md.result["TS_GET_RSP"]["t"].size(),3);
  }


  // <
  {
    auto q = njson::parse(R"(
                              {
                                "ts":"os1",
                                "rng":[0,10],
                                "where":
                                {
                                  "temp":
                                  {
                                    "<":-3
                                  }
                                }
                              }
                            )");

    MeasureDuration md {[&s, &q, rspName = GetRspCmd]{ return s.get(q, rspName).rsp; }};
    ASSERT_EQ(md.result["TS_GET_RSP"]["t"].size(), 5);
  }


  // <=
  {
    auto q = njson::parse(R"(
                              {
                                "ts":"os1",
                                "rng":[0,10],
                                "where":
                                {
                                  "temp":
                                  {
                                    "<=":-3
                                  }
                                }
                              }
                            )");

    MeasureDuration md {[&s, &q, rspName = GetRspCmd]{ return s.get(q, rspName).rsp; }};
    ASSERT_EQ(md.result["TS_GET_RSP"]["t"].size(), 6);
  }


  // []
  {
    auto q = njson::parse(R"(
                              {
                                "ts":"os1",
                                "rng":[0,10],
                                "where":
                                {
                                  "temp":
                                  {
                                    "[]":[-3,8]
                                  }
                                }
                              }
                            )");

    MeasureDuration md {[&s, &q, rspName = GetRspCmd]{ return s.get(q, rspName).rsp; }};
    ASSERT_EQ(md.result["TS_GET_RSP"]["t"].size(), 2);
  }


  // ==
  {
    auto q = njson::parse(R"(
                              {
                                "ts":"os1",
                                "rng":[0,16],
                                "where":
                                {
                                  "temp":
                                  {
                                    "==":-21
                                  }
                                }
                              }
                            )");

    MeasureDuration md {[&s, &q, rspName = GetRspCmd]{ return s.get(q, rspName).rsp; }};
    ASSERT_EQ(md.result["TS_GET_RSP"]["t"].size(), 2);
  }
  

  // == (in full range)
  {
    auto q = njson::parse(R"(
                              {
                                "ts":"os1",
                                "rng":[],
                                "where":
                                {
                                  "temp":
                                  {
                                    "==":-21
                                  }
                                }
                              }
                            )");

    MeasureDuration md {[&s, &q, rspName = GetRspCmd]{ return s.get(q, rspName).rsp; }};
    ASSERT_EQ(md.result["TS_GET_RSP"]["t"].size(), 168);
  }
}


// As Test10K_Index, but create the index *after* data is added, to force build index
TEST_F(TsSeriesTest, Test10k_CreateIndex)
{
  Series s;
  createMoreData({"../test_data/moredata_10k.json"}, s, false);


  // data added by createMoreData(), now create index
  {
    BasicMeasureDuration<TsRequestStatus> md{[CreateIndexRspCmd = CreateIndexRspCmd, &s](){ return s.createIndex("os1", "temp", CreateIndexRspCmd).status;}, "Create Index"};
    ASSERT_EQ(md.result, TsRequestStatus::Ok);
  }
  
  // == (in full range)
  {
    auto q = njson::parse(R"(
                              {
                                "ts":"os1",
                                "rng":[],
                                "where":
                                {
                                  "temp":
                                  {
                                    "==":-21
                                  }
                                }
                              }
                            )");

    MeasureDuration md {[&s, &q, rspName = GetRspCmd]{ return s.get(q, rspName).rsp; }};
    ASSERT_EQ(md.result["TS_GET_RSP"]["t"].size(), 168);
  }
}


int main (int argc, char ** argv)
{
	testing::InitGoogleTest(&argc, argv);	

	return RUN_ALL_TESTS();	
}
