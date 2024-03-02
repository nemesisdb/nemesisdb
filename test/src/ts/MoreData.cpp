//#define NDB_NOLOG

#include "../useful/TsSeriesTest.h"
#include <core/ts/OrderedSeries.h>
#include <string_view>
#include <sstream>


using namespace nemesis::test;
using namespace nemesis::core;
using namespace nemesis::core::ts;



struct MeasureDuration
{
  MeasureDuration (std::function<njson()> f)
  {
    const auto start = chrono::steady_clock::now();

    result = f();

    #ifdef NDEBUG
    PLOGI << chrono::duration_cast<chrono::microseconds> (chrono::steady_clock::now() - start).count() << '\n';
    #endif
  }

  njson result;
};



TEST_F(TsSeriesTest, Test100k)
{
  Series s;
  createMoreData({"../test_data/moredata_10k.json"}, s);
  
  
  // >
  // {
  //   auto q = njson::parse(R"(
  //                             {
  //                               "ts":["os1"],
  //                               "rng":[4000,6000],
  //                               "where":
  //                               {
  //                                 "temp":
  //                                 {
  //                                   ">":0
  //                                 }
  //                               }
  //                             }
  //                           )");

  //   MeasureDuration md {[&s, &q, rspName = GetRspCmd]{ return s.get(q, rspName).rsp; }};
  //   //std::cout << "size: " << md.result["TS_GET_RSP"]["os1"]["t"].size() << '\n';
  // }

  
  // >
  {
    auto q = njson::parse(R"(
                              {
                                "ts":["os1"],
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
    ASSERT_EQ(md.result["TS_GET_RSP"]["os1"]["t"].size(),2);
  }

  /* 
  // >=
  {
    auto q = njson::parse(R"(
                              {
                                "ts":["os1"],
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
    ASSERT_EQ(md.result["TS_GET_RSP"]["os1"]["t"].size(),3);
  }


  // <
  {
    auto q = njson::parse(R"(
                              {
                                "ts":["os1"],
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
    ASSERT_EQ(md.result["TS_GET_RSP"]["os1"]["t"].size(), 5);
  }


  // <=
  {
    auto q = njson::parse(R"(
                              {
                                "ts":["os1"],
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
    ASSERT_EQ(md.result["TS_GET_RSP"]["os1"]["t"].size(), 6);
  }


  // []
  {
    auto q = njson::parse(R"(
                              {
                                "ts":["os1"],
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
    ASSERT_EQ(md.result["TS_GET_RSP"]["os1"]["t"].size(), 2);
  }


  // ==
  {
    auto q = njson::parse(R"(
                              {
                                "ts":["os1"],
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
    ASSERT_EQ(md.result["TS_GET_RSP"]["os1"]["t"].size(), 2);
  }
  */

  // == (in full range)
  {
    auto q = njson::parse(R"(
                              {
                                "ts":["os1"],
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
    ASSERT_EQ(md.result["TS_GET_RSP"]["os1"]["t"].size(), 168);
  }
}


int main (int argc, char ** argv)
{
	testing::InitGoogleTest(&argc, argv);	

	return RUN_ALL_TESTS();	
}
