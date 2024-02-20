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
  
  {
    //njson q = njson::parse(R"( {"ts":["os1"], "rng":[0,50], "where":"$[?($.temp >= -30 && $.temp <= 30)]"} )");
    //MeasureDuration {[&s, &q, rspName = GetRspCmd]{ return s.get(q, rspName).rsp; }};
    
    ASSERT_EQ(s.create("os1", CreateRspCmd).status, TsRequestStatus::Ok);
    ASSERT_EQ(s.createIndex("os1", CreateIndexRspCmd, "temp").status, TsRequestStatus::Ok);

    auto add = njson::parse(R"(
                                {
                                  "ts":"os1",
                                  "t":[1,2,3,4,5],
                                  "v":[{"temp":10},{"temp":10},{"temp":8},{"temp":4},{"temp":2}]
                                }
                              )");
    

    s.add(add, AddRspCmd);

    auto get = njson::parse(R"(
                                {
                                  "ts":["os1"],
                                  "rng":[1,5],
                                  "where":
                                  {
                                    "temp":
                                    {
                                      ">":4
                                    }
                                  }
                                }
                              )");

    s.get(get, GetRspCmd);
  }
}


int main (int argc, char ** argv)
{
	testing::InitGoogleTest(&argc, argv);	

	return RUN_ALL_TESTS();	
}
