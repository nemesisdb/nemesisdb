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



TEST_F(TsSeriesTest, SimpleData)
{
  Series s;
  
  
  ASSERT_EQ(s.create("os1", CreateRspCmd).status, TsRequestStatus::Ok);
  ASSERT_EQ(s.createIndex("os1", "temp", CreateIndexRspCmd).status, TsRequestStatus::Ok);

  auto add = njson::parse(R"(
                              {
                                "ts":"os1",
                                "t":[10,11,12,13],
                                "evt":[{"temp":1}, {"temp":2}, {"temp":1}, {"temp":3}]
                              }
                            )");

  s.add(add, AddRspCmd);

  
  // <
  {
    auto get = njson::parse(R"(
                              {
                                "ts":"os1",
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
    
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["t"].size(), 3);
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["t"].size(), r.rsp["TS_GET_RSP"]["evt"].size());
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["t"], njson::parse(R"([10,11,12])"));
  }


  // <=
  {
    auto get = njson::parse(R"(
                              {
                                "ts":"os1",
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

    ASSERT_EQ(r.rsp["TS_GET_RSP"]["t"].size(), 2);
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["t"].size(), r.rsp["TS_GET_RSP"]["evt"].size());
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["t"], njson::parse(R"([10,12])"));
  }
  

  // >
  {
    auto get = njson::parse(R"(
                              {
                                "ts":"os1",
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

    ASSERT_EQ(r.rsp["TS_GET_RSP"]["t"].size(), 1);
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["t"].size(), r.rsp["TS_GET_RSP"]["evt"].size());
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["t"], njson::parse(R"([13])"));
  }


  // >=
  {
    auto get = njson::parse(R"(
                              {
                                "ts":"os1",
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

    ASSERT_EQ(r.rsp["TS_GET_RSP"]["t"].size(), 2);
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["t"].size(), r.rsp["TS_GET_RSP"]["evt"].size());
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["t"], njson::parse(R"([11,13])"));
  }
  

  // ==
  {
    auto get = njson::parse(R"(
                              {
                                "ts":"os1",
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

    ASSERT_EQ(r.rsp["TS_GET_RSP"]["t"].size(), 2);
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["t"].size(), r.rsp["TS_GET_RSP"]["evt"].size());
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["t"], njson::parse(R"([10,12])"));
  }


  // []
  {
    auto get = njson::parse(R"(
                              {
                                "ts":"os1",
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

    ASSERT_EQ(r.rsp["TS_GET_RSP"]["t"].size(), 3);
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["t"].size(), r.rsp["TS_GET_RSP"]["evt"].size());
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["t"], njson::parse(R"([10,11,12])"));
  }
  

  
  // []
  {
    auto get = njson::parse(R"(
                              {
                                "ts":"os1",
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

    ASSERT_EQ(r.rsp["TS_GET_RSP"]["t"].size(), 4);
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["t"].size(), r.rsp["TS_GET_RSP"]["evt"].size());
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["t"], njson::parse(R"([10,11,12,13])"));
  }


  // []
  {
    auto get = njson::parse(R"(
                              {
                                "ts":"os1",
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

    ASSERT_EQ(r.rsp["TS_GET_RSP"]["t"].size(), 3);
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["t"].size(), r.rsp["TS_GET_RSP"]["evt"].size());
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["t"], njson::parse(R"([10,11,12])"));
  }


  // []
  {
    auto get = njson::parse(R"(
                              {
                                "ts":"os1",
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

    ASSERT_EQ(r.rsp["TS_GET_RSP"]["t"].size(), 4);
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["t"].size(), r.rsp["TS_GET_RSP"]["evt"].size());
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["t"], njson::parse(R"([10,11,12,13])"));
  }
}



TEST_F(TsSeriesTest, ComplexData)
{
  // Similar to SimpleData, but values have more than just 'temp' member

  Series s;
  
  
  ASSERT_EQ(s.create("os1", CreateRspCmd).status, TsRequestStatus::Ok);
  ASSERT_EQ(s.createIndex("os1", "temp", CreateIndexRspCmd).status, TsRequestStatus::Ok);

  auto add = njson::parse(R"(
                              {
                                "ts":"os1",
                                "t":[10,11,12,13],
                                "evt":[
                                      {"temp":1, "coords":{"x":2.0, "y":5.0}},
                                      {"temp":2, "coords":{"x":2.1, "y":5.1}},
                                      {"temp":1, "coords":{"x":2.2, "y":5.2}},
                                      {"temp":3, "coords":{"x":2.3, "y":5.3}}
                                    ]
                              }
                            )");

  s.add(add, AddRspCmd);

  
  // <
  {
    auto get = njson::parse(R"(
                              {
                                "ts":"os1",
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

    ASSERT_EQ(r.rsp["TS_GET_RSP"]["t"].size(), 2);
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["t"].size(), r.rsp["TS_GET_RSP"]["evt"].size());
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["t"], njson::parse(R"([10,12])"));
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["evt"], njson::parse(R"([{"temp":1, "coords":{"x":2.0, "y":5.0}}, {"temp":1, "coords":{"x":2.2, "y":5.2}}])"));
  }
}




TEST_F(TsSeriesTest, TwoTerms)
{
  Series s;
  
  
  ASSERT_EQ(s.create("os1", CreateRspCmd).status, TsRequestStatus::Ok);
  ASSERT_EQ(s.createIndex("os1", "temp",      CreateIndexRspCmd).status, TsRequestStatus::Ok);
  ASSERT_EQ(s.createIndex("os1", "pressure",  CreateIndexRspCmd).status, TsRequestStatus::Ok);

  auto add = njson::parse(R"(
                              {
                                "ts":"os1",
                                "t":[10,11,12,13,14],
                                "evt":[{"temp":1, "pressure":10}, {"temp":2, "pressure":11}, {"temp":1, "pressure":10}, {"temp":3, "pressure":11}, {"temp":4, "pressure":12}]
                              }
                            )");

  s.add(add, AddRspCmd);

  
  // // < and ==
  {
    auto get = njson::parse(R"(
                              {
                                "ts":"os1",
                                "rng":[10,13],
                                "where":
                                {
                                  "temp":
                                  {
                                    "<":3
                                  },
                                  "pressure":
                                  {
                                    "==":10
                                  }
                                }
                              }
                            )");

    auto r = s.get(get, GetRspCmd);
    
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["t"].size(), 2);
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["t"].size(), r.rsp["TS_GET_RSP"]["evt"].size());
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["t"], njson::parse(R"([10,12])"));
  }
  

  
  // < and []
  {
    auto get = njson::parse(R"(
                              {
                                "ts":"os1",
                                "rng":[10,13],
                                "where":
                                {
                                  "temp":
                                  {
                                    "<":3
                                  },
                                  "pressure":
                                  {
                                    "[]":[10,11]
                                  }
                                }
                              }
                            )");

    auto r = s.get(get, GetRspCmd);
    
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["t"].size(), 3);
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["t"].size(), r.rsp["TS_GET_RSP"]["evt"].size());
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["t"], njson::parse(R"([10,11,12])"));
  }
 

  
  // >= and >
  {
    auto get = njson::parse(R"(
                              {
                                "ts":"os1",
                                "rng":[10,14],
                                "where":
                                {
                                  "temp":
                                  {
                                    ">=":3
                                  },
                                  "pressure":
                                  {
                                    ">":11
                                  }
                                }
                              }
                            )");

    auto r = s.get(get, GetRspCmd);
    
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["t"].size(), 1);
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["t"].size(), r.rsp["TS_GET_RSP"]["evt"].size());
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["t"], njson::parse(R"([14])"));
  }
  

  // >= and > (as above, but with lower max rng)
  {
    auto get = njson::parse(R"(
                              {
                                "ts":"os1",
                                "rng":[10,13],
                                "where":
                                {
                                  "temp":
                                  {
                                    ">=":3
                                  },
                                  "pressure":
                                  {
                                    ">":11
                                  }
                                }
                              }
                            )");

    auto r = s.get(get, GetRspCmd);
    
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["t"].size(), 0);
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["t"].size(), r.rsp["TS_GET_RSP"]["evt"].size());
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["t"], njson::parse(R"([])"));
  }
}



TEST_F(TsSeriesTest, BuildIndex)
{
  // add data then create index (the above tests create index before adding data)
  Series s;
  
  
  ASSERT_EQ(s.create("os1", CreateRspCmd).status, TsRequestStatus::Ok);
  
  {
    auto add = njson::parse(R"(
                              {
                                "ts":"os1",
                                "t":[10,11,12,13],
                                "evt":[{"temp":1}, {"temp":2}, {"temp":1}, {"temp":3}]
                              }
                            )");

    s.add(add, AddRspCmd);
  }
  

  ASSERT_EQ(s.createIndex("os1", "temp", CreateIndexRspCmd).status, TsRequestStatus::Ok);


  {
    auto get = njson::parse(R"(
                              {
                                "ts":"os1",
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
    
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["t"].size(), 3);
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["t"].size(), r.rsp["TS_GET_RSP"]["evt"].size());
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["t"], njson::parse(R"([10,11,12])"));
  }
  


  // add more data
  {
    auto add = njson::parse(R"(
                              {
                                "ts":"os1",
                                "t":[14,15,16,17],
                                "evt":[{"temp":3}, {"temp":1}, {"temp":8}, {"temp":7}]
                              }
                            )");

    s.add(add, AddRspCmd);
  }

  {
    auto get = njson::parse(R"(
                              {
                                "ts":"os1",
                                "rng":[],
                                "where":
                                {
                                  "temp":
                                  {
                                    "[]":[3,7]
                                  }
                                }
                              }
                            )");

    auto r = s.get(get, GetRspCmd);
    
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["t"].size(), 3);
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["t"].size(), r.rsp["TS_GET_RSP"]["evt"].size());
    ASSERT_EQ(r.rsp["TS_GET_RSP"]["t"], njson::parse(R"([13,14,17])"));
  }
}



int main (int argc, char ** argv)
{
	testing::InitGoogleTest(&argc, argv);	

	return RUN_ALL_TESTS();	
}

