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
    std::cout << chrono::duration_cast<chrono::microseconds> (chrono::steady_clock::now() - start).count() << '\n';
    #endif
  }

  njson result;
};



TEST_F(TsSeriesTest, Create)
{
  Series s;
  
  
  ASSERT_EQ(s.create("os1", CreateRspCmd).status, TsRequestStatus::Ok);
  ASSERT_EQ(s.createIndex("os1", "temp", CreateIndexRspCmd).status, TsRequestStatus::Ok);

  auto add = njson::parse(R"(
                              {
                                "ts":"os1",
                                "t":[10,11,12,13],
                                "v":[{"temp":1}, {"temp":2}, {"temp":1}, {"temp":3}]
                              }
                            )");

  s.add(add, AddRspCmd);

  
  // <
  {
    auto get = njson::parse(R"(
                              {
                                "ts":["os1"],
                                "rng":[10,13],
                                "where":
                                {
                                  "temp":
                                  {
                                    "<":3
                                  }
                                }
                              }
                            )");

    auto r = s.get(get, GetRspCmd);

    std::cout << r.rsp << '\n';

    ASSERT_EQ(r.rsp["TS_GET_RSP"]["os1"]["t"].size(), 3);
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["os1"]["t"].size(), r.rsp["TS_GET_RSP"]["os1"]["v"].size());
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["os1"]["t"], njson::parse(R"([10,11,12])"));
  }


  // <=
  {
    auto get = njson::parse(R"(
                              {
                                "ts":["os1"],
                                "rng":[10,13],
                                "where":
                                {
                                  "temp":
                                  {
                                    "<=":1
                                  }
                                }
                              }
                            )");

    auto r = s.get(get, GetRspCmd);

    std::cout << r.rsp << '\n';

    ASSERT_EQ(r.rsp["TS_GET_RSP"]["os1"]["t"].size(), 2);
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["os1"]["t"].size(), r.rsp["TS_GET_RSP"]["os1"]["v"].size());
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["os1"]["t"], njson::parse(R"([10,12])"));
  }
  

  // >
  {
    auto get = njson::parse(R"(
                              {
                                "ts":["os1"],
                                "rng":[10,13],
                                "where":
                                {
                                  "temp":
                                  {
                                    ">":2
                                  }
                                }
                              }
                            )");

    auto r = s.get(get, GetRspCmd);

    std::cout << r.rsp << '\n';

    ASSERT_EQ(r.rsp["TS_GET_RSP"]["os1"]["t"].size(), 1);
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["os1"]["t"].size(), r.rsp["TS_GET_RSP"]["os1"]["v"].size());
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["os1"]["t"], njson::parse(R"([13])"));
  }


  // >=
  {
    auto get = njson::parse(R"(
                              {
                                "ts":["os1"],
                                "rng":[10,13],
                                "where":
                                {
                                  "temp":
                                  {
                                    ">=":2
                                  }
                                }
                              }
                            )");

    auto r = s.get(get, GetRspCmd);

    std::cout << r.rsp << '\n';

    ASSERT_EQ(r.rsp["TS_GET_RSP"]["os1"]["t"].size(), 2);
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["os1"]["t"].size(), r.rsp["TS_GET_RSP"]["os1"]["v"].size());
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["os1"]["t"], njson::parse(R"([11,13])"));
  }
  

  // ==
  {
    auto get = njson::parse(R"(
                              {
                                "ts":["os1"],
                                "rng":[10,13],
                                "where":
                                {
                                  "temp":
                                  {
                                    "==":1
                                  }
                                }
                              }
                            )");

    auto r = s.get(get, GetRspCmd);

    std::cout << r.rsp << '\n';

    ASSERT_EQ(r.rsp["TS_GET_RSP"]["os1"]["t"].size(), 2);
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["os1"]["t"].size(), r.rsp["TS_GET_RSP"]["os1"]["v"].size());
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["os1"]["t"], njson::parse(R"([10,12])"));
  }


  // []
  {
    auto get = njson::parse(R"(
                              {
                                "ts":["os1"],
                                "rng":[10,13],
                                "where":
                                {
                                  "temp":
                                  {
                                    "[]":[1,2]
                                  }
                                }
                              }
                            )");

    auto r = s.get(get, GetRspCmd);

    std::cout << r.rsp << '\n';

    ASSERT_EQ(r.rsp["TS_GET_RSP"]["os1"]["t"].size(), 3);
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["os1"]["t"].size(), r.rsp["TS_GET_RSP"]["os1"]["v"].size());
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["os1"]["t"], njson::parse(R"([10,11,12])"));
  }
  

  
  // []
  {
    auto get = njson::parse(R"(
                              {
                                "ts":["os1"],
                                "rng":[10,13],
                                "where":
                                {
                                  "temp":
                                  {
                                    "[]":[1,3]
                                  }
                                }
                              }
                            )");

    auto r = s.get(get, GetRspCmd);

    std::cout << r.rsp << '\n';

    ASSERT_EQ(r.rsp["TS_GET_RSP"]["os1"]["t"].size(), 4);
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["os1"]["t"].size(), r.rsp["TS_GET_RSP"]["os1"]["v"].size());
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["os1"]["t"], njson::parse(R"([10,11,12,13])"));
  }


  // []
  {
    auto get = njson::parse(R"(
                              {
                                "ts":["os1"],
                                "rng":[10,13],
                                "where":
                                {
                                  "temp":
                                  {
                                    "[]":[1,2]
                                  }
                                }
                              }
                            )");

    auto r = s.get(get, GetRspCmd);

    std::cout << r.rsp << '\n';

    ASSERT_EQ(r.rsp["TS_GET_RSP"]["os1"]["t"].size(), 3);
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["os1"]["t"].size(), r.rsp["TS_GET_RSP"]["os1"]["v"].size());
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["os1"]["t"], njson::parse(R"([10,11,12])"));
  }


  // []
  {
    auto get = njson::parse(R"(
                              {
                                "ts":["os1"],
                                "rng":[10,13],
                                "where":
                                {
                                  "temp":
                                  {
                                    "[]":[1,6]
                                  }
                                }
                              }
                            )");

    auto r = s.get(get, GetRspCmd);

    std::cout << r.rsp << '\n';

    ASSERT_EQ(r.rsp["TS_GET_RSP"]["os1"]["t"].size(), 4);
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["os1"]["t"].size(), r.rsp["TS_GET_RSP"]["os1"]["v"].size());
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["os1"]["t"], njson::parse(R"([10,11,12,13])"));
  }
}


int main (int argc, char ** argv)
{
	testing::InitGoogleTest(&argc, argv);	

	return RUN_ALL_TESTS();	
}
