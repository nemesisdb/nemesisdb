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

    std::cout << query << '\n';

    ASSERT_EQ(s.add(query.at("TS_ADD")).status, TsStatus::Ok);
  }
};


TEST_F(TsSeriesTest, GetMultiple)
{
  Series s;

  ASSERT_TRUE(s.create("os1"));
  ASSERT_TRUE(s.containsSeries("os1"));

  addSimpleData("os1", s);

  
  njson rspEverything;
  
  {
    const auto query = njson::parse(R"( { "TS_GET":{"ts":"os1", "rng":[1,5]} } )");
    const auto res = s.get(query.at("TS_GET"));
    
    ASSERT_EQ(res.status, TsStatus::Ok);
    ASSERT_TRUE(res.rsp.contains("t"));
    ASSERT_TRUE(res.rsp.contains("v"));

    ASSERT_EQ(res.rsp.at("t").size(), 4);
    ASSERT_EQ(res.rsp.at("t").size(), res.rsp.at("v").size());
    
    std::cout << res.rsp << '\n';

    rspEverything = res.rsp;
  }

  // same as before but no end set
  {
    const auto query1 = njson::parse(R"( { "TS_GET":{"ts":"os1", "rng":[1]} } )");
    const auto res = s.get(query1.at("TS_GET"));
    
    ASSERT_EQ(res.status, TsStatus::Ok);
    ASSERT_EQ(res.rsp, rspEverything);
  }
  
}


int main (int argc, char ** argv)
{
	testing::InitGoogleTest(&argc, argv);	

	return RUN_ALL_TESTS();	
}
